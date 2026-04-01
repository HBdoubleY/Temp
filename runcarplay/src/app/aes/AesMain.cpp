
#include "DHCryptlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//#if defined(__cplusplus)
//extern "C"
//{
//#endif

#define _MaxText_Length_ 256  //最大加解密数据长度 


mh_decl void xor_block_aligned(void *r, const void *p, const void *q)
{
	rep3_u4(f_xor, UNIT_PTR(r), UNIT_PTR(p), UNIT_PTR(q), UNIT_VAL);
}

gf_decl void gf_mulx1_lb(gf_t r, const gf_t x)
{   
	gf_unit_t _tt;
	_tt = gf_tab[(UNIT_PTR(x)[3] >> 17) & MASK(0x80)];

	rep2_d4(f1_lb, UNIT_PTR(r), UNIT_PTR(x));
	UNIT_PTR(r)[0] ^= _tt;
}
void init_4k_table(const gf_t g, gf_t4k_t t)
{   
	int j, k;

	memset(t[0], 0, GF_BYTE_LEN);

	memcpy(t[128], g, GF_BYTE_LEN);
	for(j = 64; j >= 1; j >>= 1)
		gf_mulx1(mode)(t[j], t[j + j]);
	for(j = 2; j < 256; j += j)
		for(k = 1; k < j; ++k)
			xor_block_aligned(t[j + k], t[j], t[k]);
}

#define xor_4k(i,ap,t,r) gf_mulx8(mode)(r); xor_block_aligned(r, r, t[ap[GF_INDEX(i)]])

#  define ls_box(x,c)       four_tables(x,t_use(f,l),vf1,rf2,c)
#define ke4(k,i) \
{   k[4*(i)+4] = ss[0] ^= ls_box(ss[3],3) ^ t_use(r,c)[i]; \
	k[4*(i)+5] = ss[1] ^= ss[0]; \
	k[4*(i)+6] = ss[2] ^= ss[1]; \
	k[4*(i)+7] = ss[3] ^= ss[2]; \
}
#define v(n,i)  ((n) - (i) + 2 * ((i) & 3))
#define k4e(k,i) \
{   k[v(40,(4*(i))+4)] = ss[0] ^= ls_box(ss[3],3) ^ t_use(r,c)[i]; \
	k[v(40,(4*(i))+5)] = ss[1] ^= ss[0]; \
	k[v(40,(4*(i))+6)] = ss[2] ^= ss[1]; \
	k[v(40,(4*(i))+7)] = ss[3] ^= ss[2]; \
}


#define kdf4(k,i) \
{   ss[0] = ss[0] ^ ss[2] ^ ss[1] ^ ss[3]; \
	ss[1] = ss[1] ^ ss[3]; \
	ss[2] = ss[2] ^ ss[3]; \
	ss[4] = ls_box(ss[(i+3) % 4], 3) ^ t_use(r,c)[i]; \
	ss[i % 4] ^= ss[4]; \
	ss[4] ^= k[v(40,(4*(i)))];   k[v(40,(4*(i))+4)] = ff(ss[4]); \
	ss[4] ^= k[v(40,(4*(i))+1)]; k[v(40,(4*(i))+5)] = ff(ss[4]); \
	ss[4] ^= k[v(40,(4*(i))+2)]; k[v(40,(4*(i))+6)] = ff(ss[4]); \
	ss[4] ^= k[v(40,(4*(i))+3)]; k[v(40,(4*(i))+7)] = ff(ss[4]); \
}

#define kd4(k,i) \
{   ss[4] = ls_box(ss[(i+3) % 4], 3) ^ t_use(r,c)[i]; \
	ss[i % 4] ^= ss[4]; ss[4] = ff(ss[4]); \
	k[v(40,(4*(i))+4)] = ss[4] ^= k[v(40,(4*(i)))]; \
	k[v(40,(4*(i))+5)] = ss[4] ^= k[v(40,(4*(i))+1)]; \
	k[v(40,(4*(i))+6)] = ss[4] ^= k[v(40,(4*(i))+2)]; \
	k[v(40,(4*(i))+7)] = ss[4] ^= k[v(40,(4*(i))+3)]; \
}

#define kdl4(k,i) \
{   ss[4] = ls_box(ss[(i+3) % 4], 3) ^ t_use(r,c)[i]; ss[i % 4] ^= ss[4]; \
	k[v(40,(4*(i))+4)] = (ss[0] ^= ss[1]) ^ ss[2] ^ ss[3]; \
	k[v(40,(4*(i))+5)] = ss[1] ^ ss[3]; \
	k[v(40,(4*(i))+6)] = ss[0]; \
	k[v(40,(4*(i))+7)] = ss[1]; \
}

AES_RETURN aes_encrypt_key128(const unsigned char *key, aes_encrypt_ctx cx[1])
{  
	uint_32t    ss[4];

	cx->ks[0] = ss[0] = word_in(key, 0);
	cx->ks[1] = ss[1] = word_in(key, 1);
	cx->ks[2] = ss[2] = word_in(key, 2);
	cx->ks[3] = ss[3] = word_in(key, 3);

	ke4(cx->ks, 0);  ke4(cx->ks, 1);
	ke4(cx->ks, 2);  ke4(cx->ks, 3);
	ke4(cx->ks, 4);  ke4(cx->ks, 5);
	ke4(cx->ks, 6);  ke4(cx->ks, 7);
	ke4(cx->ks, 8);

	ke4(cx->ks, 9);
	cx->inf.l = 0;
	cx->inf.b[0] = 10 * 16;


	cx->inf.b[1] = 0xff;

	return EXIT_SUCCESS;
}
//初始化key
AES_RETURN aes_encrypt_key(const unsigned char *key, int key_len, aes_encrypt_ctx cx[1])
{   
	if(key_len!=16&&key_len!=128)
		return EXIT_FAILURE;
    return aes_encrypt_key128(key, cx);
}
#  define s(x,c) x[c]
#define si(y,x,k,c) (s(y,c) = word_in(x, c) ^ (k)[c])
#define so(y,x,c)   word_out(y, c, s(x,c))
#define locals(y,x)     x[4],y[4]
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
	s(y,2) = s(x,2); s(y,3) = s(x,3);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)
#define fwd_var(x,r,c)\
	( r == 0 ? ( c == 0 ? s(x,0) : c == 1 ? s(x,1) : c == 2 ? s(x,2) : s(x,3))\
	: r == 1 ? ( c == 0 ? s(x,1) : c == 1 ? s(x,2) : c == 2 ? s(x,3) : s(x,0))\
	: r == 2 ? ( c == 0 ? s(x,2) : c == 1 ? s(x,3) : c == 2 ? s(x,0) : s(x,1))\
	:          ( c == 0 ? s(x,3) : c == 1 ? s(x,0) : c == 2 ? s(x,1) : s(x,2)))
#define fwd_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_tables(x,t_use(f,n),fwd_var,rf1,c))
#define fwd_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_tables(x,t_use(f,l),fwd_var,rf1,c))

AES_RETURN aes_encrypt(const unsigned char *in, unsigned char *out, const aes_encrypt_ctx cx[1])
{   
	uint_32t         locals(b0, b1);
    const uint_32t   *kp;


if( cx->inf.b[0] != 10 * 16 && cx->inf.b[0] != 12 * 16 && cx->inf.b[0] != 14 * 16 )
	return EXIT_FAILURE;

kp = cx->ks;
state_in(b0, in, kp);

switch(cx->inf.b[0])
{
case 14 * 16:
	round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
	round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
	kp += 2 * N_COLS;
case 12 * 16:
	round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
	round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
	kp += 2 * N_COLS;
case 10 * 16:
	round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
	round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
	round(fwd_rnd,  b1, b0, kp + 3 * N_COLS);
	round(fwd_rnd,  b0, b1, kp + 4 * N_COLS);
	round(fwd_rnd,  b1, b0, kp + 5 * N_COLS);
	round(fwd_rnd,  b0, b1, kp + 6 * N_COLS);
	round(fwd_rnd,  b1, b0, kp + 7 * N_COLS);
	round(fwd_rnd,  b0, b1, kp + 8 * N_COLS);
	round(fwd_rnd,  b1, b0, kp + 9 * N_COLS);
	round(fwd_lrnd, b0, b1, kp +10 * N_COLS);
}
state_out(out, b0);
return EXIT_SUCCESS;
}


#define inc_ctr(x)  \
{   int i = BLOCK_SIZE; while(i-- > CTR_POS && !++(UI8_PTR(x)[i])) ; }

ret_type gcm_init_and_key(          /* initialise mode and set key  */
	const unsigned char key[],      /* the key value                */
	unsigned long key_len,          /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{
	memset(ctx->ghash_h, 0, sizeof(ctx->ghash_h));

	/* set the AES key                          */
	aes_encrypt_key(key, key_len, ctx->aes);

	/* compute E(0) (for the hash function)     */
	aes_encrypt(UI8_PTR(ctx->ghash_h), UI8_PTR(ctx->ghash_h), ctx->aes);

#if defined( TABLES_4K )
	init_4k_table(ctx->ghash_h, ctx->gf_t4k);
#endif
	return RETURN_GOOD;
}
mh_decl void copy_block_aligned(void *p, const void *q)
{
	 rep2_u4(f_copy,UNIT_PTR(p),UNIT_PTR(q));
}
gf_decl void gf_mulx8_lb(gf_t x)
{   gf_unit_t _tt;

   _tt = gf_tab[UNIT_PTR(x)[3] >> 24];

   rep2_d4(f8_lb, UNIT_PTR(x), UNIT_PTR(x));
   UNIT_PTR(x)[0] ^= _tt;
}

#define xor_4k(i,ap,t,r) gf_mulx8(mode)(r); xor_block_aligned(r, r, t[ap[GF_INDEX(i)]])
void gf_mul_4k(gf_t a, const gf_t4k_t t, gf_t r)
{   
	uint_8t *ap = (uint_8t*)a;
	memset(r, 0, GF_BYTE_LEN);
	xor_4k(15, ap, t, r); xor_4k(14, ap, t, r);
	xor_4k(13, ap, t, r); xor_4k(12, ap, t, r);
	xor_4k(11, ap, t, r); xor_4k(10, ap, t, r);
	xor_4k( 9, ap, t, r); xor_4k( 8, ap, t, r);
	xor_4k( 7, ap, t, r); xor_4k( 6, ap, t, r);
	xor_4k( 5, ap, t, r); xor_4k( 4, ap, t, r);
	xor_4k( 3, ap, t, r); xor_4k( 2, ap, t, r);
	xor_4k( 1, ap, t, r); xor_4k( 0, ap, t, r);
	copy_block_aligned(a, r);
}
void gf_mul_hh(gf_t a, gcm_ctx ctx[1])
{
	gf_t    scr;
	gf_mul_4k(a, ctx->gf_t4k, scr);
}
ret_type gcm_init_message(                  /* initialise a new message     */
	const unsigned char iv[],       /* the initialisation vector    */
	unsigned long iv_len,           /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{   
	uint_32t i, n_pos = 0;
	uint_8t *p;

	memset(ctx->ctr_val, 0, BLOCK_SIZE);
	if(iv_len == CTR_POS)
	{
		memcpy(ctx->ctr_val, iv, CTR_POS); UI8_PTR(ctx->ctr_val)[15] = 0x01;
	}
	else
	{   n_pos = iv_len;
	while(n_pos >= BLOCK_SIZE)
	{
		xor_block_aligned(ctx->ctr_val, ctx->ctr_val, iv);
		n_pos -= BLOCK_SIZE;
		iv += BLOCK_SIZE;
		gf_mul_hh(ctx->ctr_val, ctx);
	}

	if(n_pos)
	{
		p = UI8_PTR(ctx->ctr_val);
		while(n_pos-- > 0)
			*p++ ^= *iv++;
		gf_mul_hh(ctx->ctr_val, ctx);
	}
	n_pos = (iv_len << 3);
	for(i = BLOCK_SIZE - 1; n_pos; --i, n_pos >>= 8)
		UI8_PTR(ctx->ctr_val)[i] ^= (unsigned char)n_pos;
	gf_mul_hh(ctx->ctr_val, ctx);
	}

	ctx->y0_val = *UI32_PTR(UI8_PTR(ctx->ctr_val) + CTR_POS);
	memset(ctx->hdr_ghv, 0, BLOCK_SIZE);
	memset(ctx->txt_ghv, 0, BLOCK_SIZE);
	ctx->hdr_cnt = 0;
	ctx->txt_ccnt = ctx->txt_acnt = 0;
	return RETURN_GOOD;
}
mh_decl void xor_block(void *r, const void* p, const void* q)
{
	rep3_u16(f_xor, UI8_PTR(r), UI8_PTR(p), UI8_PTR(q), UI8_VAL);
}

ret_type gcm_auth_header(                   /* authenticate the header      */
	const unsigned char hdr[],      /* the header buffer            */
	unsigned long hdr_len,          /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{  
	uint_32t cnt = 0, b_pos = (uint_32t)ctx->hdr_cnt & BLK_ADR_MASK;

	if(!hdr_len)
		return RETURN_GOOD;

	if(ctx->hdr_cnt && b_pos == 0)
		gf_mul_hh(ctx->hdr_ghv, ctx);

	if(!((hdr - (UI8_PTR(ctx->hdr_ghv) + b_pos)) & BUF_ADRMASK))
	{
		while(cnt < hdr_len && (b_pos & BUF_ADRMASK))
			UI8_PTR(ctx->hdr_ghv)[b_pos++] ^= hdr[cnt++];

		while(cnt + BUF_INC <= hdr_len && b_pos <= BLOCK_SIZE - BUF_INC)
		{
			*UNIT_PTR(UI8_PTR(ctx->hdr_ghv) + b_pos) ^= *UNIT_PTR(hdr + cnt);
			cnt += BUF_INC; b_pos += BUF_INC;
		}

		while(cnt + BLOCK_SIZE <= hdr_len)
		{
			gf_mul_hh(ctx->hdr_ghv, ctx);
			xor_block_aligned(ctx->hdr_ghv, ctx->hdr_ghv, hdr + cnt);
			cnt += BLOCK_SIZE;
		}
	}
	else
	{
		while(cnt < hdr_len && b_pos < BLOCK_SIZE)
			UI8_PTR(ctx->hdr_ghv)[b_pos++] ^= hdr[cnt++];

		while(cnt + BLOCK_SIZE <= hdr_len)
		{
			gf_mul_hh(ctx->hdr_ghv, ctx);
			xor_block(ctx->hdr_ghv, ctx->hdr_ghv, hdr + cnt);
			cnt += BLOCK_SIZE;
		}
	}

	while(cnt < hdr_len)
	{
		if(b_pos == BLOCK_SIZE)
		{
			gf_mul_hh(ctx->hdr_ghv, ctx);
			b_pos = 0;
		}
		UI8_PTR(ctx->hdr_ghv)[b_pos++] ^= hdr[cnt++];
	}

	ctx->hdr_cnt += cnt;
	return RETURN_GOOD;
}

//////////////////////////////////////////////////////////////////////////

ret_type gcm_crypt_data(                    /* encrypt or decrypt data      */
	unsigned char data[],           /* the data buffer              */
	unsigned long data_len,         /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{   
	uint_32t cnt = 0, b_pos = (uint_32t)ctx->txt_ccnt & BLK_ADR_MASK;

	if(!data_len)
		return RETURN_GOOD;

	if(!((data - (UI8_PTR(ctx->enc_ctr) + b_pos)) & BUF_ADRMASK))
	{
		if(b_pos)
		{
			while(cnt < data_len && (b_pos & BUF_ADRMASK))
				data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];

			while(cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
			{
				*UNIT_PTR(data + cnt) ^= *UNIT_PTR(UI8_PTR(ctx->enc_ctr) + b_pos);
				cnt += BUF_INC; b_pos += BUF_INC;
			}
		}

		while(cnt + BLOCK_SIZE <= data_len)
		{
			inc_ctr(ctx->ctr_val);
			aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
			xor_block_aligned(data + cnt, data + cnt, ctx->enc_ctr);
			cnt += BLOCK_SIZE;
		}
	}
	else
	{
		if(b_pos)
			while(cnt < data_len && b_pos < BLOCK_SIZE)
				data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];

		while(cnt + BLOCK_SIZE <= data_len)
		{
			inc_ctr(ctx->ctr_val);
			aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
			xor_block(data + cnt, data + cnt, ctx->enc_ctr);
			cnt += BLOCK_SIZE;
		}
	}

	while(cnt < data_len)
	{
		if(b_pos == BLOCK_SIZE || !b_pos)
		{
			inc_ctr(ctx->ctr_val);
			aes_encrypt(UI8_PTR(ctx->ctr_val), UI8_PTR(ctx->enc_ctr), ctx->aes);
			b_pos = 0;
		}
		data[cnt++] ^= UI8_PTR(ctx->enc_ctr)[b_pos++];
	}

	ctx->txt_ccnt += cnt;
	return RETURN_GOOD;
}

ret_type gcm_auth_data(                     /* authenticate ciphertext data */
	const unsigned char data[],     /* the data buffer              */
	unsigned long data_len,         /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{   
	uint_32t cnt = 0, b_pos = (uint_32t)ctx->txt_acnt & BLK_ADR_MASK;

	if(!data_len)
		return RETURN_GOOD;

	if(ctx->txt_acnt && b_pos == 0)
		gf_mul_hh(ctx->txt_ghv, ctx);

	if(!((data - (UI8_PTR(ctx->txt_ghv) + b_pos)) & BUF_ADRMASK))
	{
		while(cnt < data_len && (b_pos & BUF_ADRMASK))
			UI8_PTR(ctx->txt_ghv)[b_pos++] ^= data[cnt++];

		while(cnt + BUF_INC <= data_len && b_pos <= BLOCK_SIZE - BUF_INC)
		{
			*UNIT_PTR(UI8_PTR(ctx->txt_ghv) + b_pos) ^= *UNIT_PTR(data + cnt);
			cnt += BUF_INC; b_pos += BUF_INC;
		}

		while(cnt + BLOCK_SIZE <= data_len)
		{
			gf_mul_hh(ctx->txt_ghv, ctx);
			xor_block_aligned(ctx->txt_ghv, ctx->txt_ghv, data + cnt);
			cnt += BLOCK_SIZE;
		}
	}
	else
	{
		while(cnt < data_len && b_pos < BLOCK_SIZE)
			UI8_PTR(ctx->txt_ghv)[b_pos++] ^= data[cnt++];

		while(cnt + BLOCK_SIZE <= data_len)
		{
			gf_mul_hh(ctx->txt_ghv, ctx);
			xor_block(ctx->txt_ghv, ctx->txt_ghv, data + cnt);
			cnt += BLOCK_SIZE;
		}
	}

	while(cnt < data_len)
	{
		if(b_pos == BLOCK_SIZE)
		{
			gf_mul_hh(ctx->txt_ghv, ctx);
			b_pos = 0;
		}
		UI8_PTR(ctx->txt_ghv)[b_pos++] ^= data[cnt++];
	}

	ctx->txt_acnt += cnt;
	return RETURN_GOOD;
}

ret_type gcm_encrypt(                       /* encrypt & authenticate data  */
	unsigned char data[],           /* the data buffer              */
	unsigned long data_len,         /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{
	gcm_crypt_data(data, data_len, ctx);
	gcm_auth_data(data, data_len, ctx);
	return RETURN_GOOD;
}


/* A slow field multiplier */

void gf_mul(gf_t a, const gf_t b)
{   
	gf_t p[8];
	uint_8t *q, ch;
	int i;

	copy_block_aligned(p[0], a);
	for(i = 0; i < 7; ++i)
		gf_mulx1(mode)(p[i + 1], p[i]);

	q = (uint_8t*)(a == b ? p[0] : b);
	memset(a, 0, GF_BYTE_LEN);
	for(i = 15 ;  ; )
	{
		ch = q[GF_INDEX(i)];
		if(ch & X_0)
			xor_block_aligned(a, a, p[0]);
		if(ch & X_1)
			xor_block_aligned(a, a, p[1]);
		if(ch & X_2)
			xor_block_aligned(a, a, p[2]);
		if(ch & X_3)
			xor_block_aligned(a, a, p[3]);
		if(ch & X_4)
			xor_block_aligned(a, a, p[4]);
		if(ch & X_5)
			xor_block_aligned(a, a, p[5]);
		if(ch & X_6)
			xor_block_aligned(a, a, p[6]);
		if(ch & X_7)
			xor_block_aligned(a, a, p[7]);
		if(!i--)
			break;
		gf_mulx8(mode)(a);
	}
}

ret_type gcm_compute_tag(                   /* compute authentication tag   */
	unsigned char tag[],            /* the buffer for the tag       */
	unsigned long tag_len,          /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{  
	uint_32t i, ln;
	gf_t tbuf;

	if(ctx->txt_acnt != ctx->txt_ccnt && ctx->txt_ccnt > 0)
		return RETURN_ERROR;

	gf_mul_hh(ctx->hdr_ghv, ctx);
	gf_mul_hh(ctx->txt_ghv, ctx);

	if(ctx->hdr_cnt)
	{
		ln = (uint_32t)((ctx->txt_acnt + BLOCK_SIZE - 1) / BLOCK_SIZE);
		if(ln)
		{
			/* alternative versions of the exponentiation operation */
			memcpy(tbuf, ctx->ghash_h, BLOCK_SIZE);

			for( ; ; )
			{
				if(ln & 1)
				{
					gf_mul(ctx->hdr_ghv, tbuf);
				}
				if(!(ln >>= 1))
					break;
				gf_mul(tbuf, tbuf);
			}
		}
	}

	i = BLOCK_SIZE; 

	{   uint_64t tm = ((uint_64t)ctx->txt_acnt) << 3;
	while(i-- > 0)
	{
		UI8_PTR(ctx->hdr_ghv)[i] ^= UI8_PTR(ctx->txt_ghv)[i] ^ (unsigned char)tm;
		tm = (i == 8 ? (((uint_64t)ctx->hdr_cnt) << 3) : tm >> 8);
	}
	}


	gf_mul_hh(ctx->hdr_ghv, ctx);

	memcpy(ctx->enc_ctr, ctx->ctr_val, BLOCK_SIZE);
	*UI32_PTR(UI8_PTR(ctx->enc_ctr) + CTR_POS) = ctx->y0_val;
	aes_encrypt(UI8_PTR(ctx->enc_ctr), UI8_PTR(ctx->enc_ctr), ctx->aes);
	for(i = 0; i < (unsigned int)tag_len; ++i)
		tag[i] = (unsigned char)(UI8_PTR(ctx->hdr_ghv)[i] ^ UI8_PTR(ctx->enc_ctr)[i]);

	return (ctx->txt_ccnt == ctx->txt_acnt ? RETURN_GOOD : RETURN_WARN);
}

ret_type gcm_end(                           /* clean up and end operation   */
	gcm_ctx ctx[1])                 /* the mode context             */
{
	memset(ctx, 0, sizeof(gcm_ctx));
	return RETURN_GOOD;
}
//////////////////////////////////////////////////////////////////////////
ret_type gcm_decrypt(                       /* authenticate & decrypt data  */
	unsigned char data[],           /* the data buffer              */
	unsigned long data_len,         /* and its length in bytes      */
	gcm_ctx ctx[1])                 /* the mode context             */
{
	gcm_auth_data(data, data_len, ctx);
	gcm_crypt_data(data, data_len, ctx);
	return RETURN_GOOD;
}

AES_RETURN aes_decrypt_key128(const unsigned char *key, aes_decrypt_ctx cx[1])
{   
	uint_32t    ss[5];
	cx->ks[v(40,(0))] = ss[0] = word_in(key, 0);
	cx->ks[v(40,(1))] = ss[1] = word_in(key, 1);
	cx->ks[v(40,(2))] = ss[2] = word_in(key, 2);
	cx->ks[v(40,(3))] = ss[3] = word_in(key, 3);
	kdf4(cx->ks, 0); kd4(cx->ks, 1);
	kd4(cx->ks, 2);  kd4(cx->ks, 3);
	kd4(cx->ks, 4);  kd4(cx->ks, 5);
	kd4(cx->ks, 6);  kd4(cx->ks, 7);
	kd4(cx->ks, 8);  kdl4(cx->ks, 9);
	cx->inf.l = 0;
	cx->inf.b[0] = 10 * 16;
	cx->inf.b[1] = 0xff;
	return EXIT_SUCCESS;
}
#define key_ofs     0
#define rnd_key(n)  (kp + n * N_COLS)
#define inv_var(x,r,c)\
	( r == 0 ? ( c == 0 ? s(x,0) : c == 1 ? s(x,1) : c == 2 ? s(x,2) : s(x,3))\
	: r == 1 ? ( c == 0 ? s(x,3) : c == 1 ? s(x,0) : c == 2 ? s(x,1) : s(x,2))\
	: r == 2 ? ( c == 0 ? s(x,2) : c == 1 ? s(x,3) : c == 2 ? s(x,0) : s(x,1))\
	:          ( c == 0 ? s(x,1) : c == 1 ? s(x,2) : c == 2 ? s(x,3) : s(x,0)))
#define inv_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_tables(x,t_use(i,n),inv_var,rf1,c))
#define inv_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_tables(x,t_use(i,l),inv_var,rf1,c))
AES_RETURN aes_decrypt(const unsigned char *in, unsigned char *out, const aes_decrypt_ctx cx[1])
{
	uint_32t        locals(b0, b1);
	const uint_32t *kp;

	if( cx->inf.b[0] != 10 * 16 && cx->inf.b[0] != 12 * 16 && cx->inf.b[0] != 14 * 16 )
		return EXIT_FAILURE;

	kp = cx->ks + (key_ofs ? (cx->inf.b[0] >> 2) : 0);
	
	state_in(b0, in, kp);
	kp = cx->ks + (key_ofs ? 0 : (cx->inf.b[0] >> 2));
	switch(cx->inf.b[0])
	{
	case 14 * 16:
		round(inv_rnd,  b1, b0, rnd_key(-13));
		round(inv_rnd,  b0, b1, rnd_key(-12));
	case 12 * 16:
		round(inv_rnd,  b1, b0, rnd_key(-11));
		round(inv_rnd,  b0, b1, rnd_key(-10));
	case 10 * 16:
		round(inv_rnd,  b1, b0, rnd_key(-9));
		round(inv_rnd,  b0, b1, rnd_key(-8));
		round(inv_rnd,  b1, b0, rnd_key(-7));
		round(inv_rnd,  b0, b1, rnd_key(-6));
		round(inv_rnd,  b1, b0, rnd_key(-5));
		round(inv_rnd,  b0, b1, rnd_key(-4));
		round(inv_rnd,  b1, b0, rnd_key(-3));
		round(inv_rnd,  b0, b1, rnd_key(-2));
		round(inv_rnd,  b1, b0, rnd_key(-1));
		round(inv_lrnd, b0, b1, rnd_key( 0));
	}
	state_out(out, b0);
	return EXIT_SUCCESS;
}
AES_RETURN aes_decrypt_key(const unsigned char *key, int key_len, aes_decrypt_ctx cx[1])
{
	switch(key_len)
	{
	case 16: case 128: return aes_decrypt_key128(key, cx);
	default: return EXIT_FAILURE;
	}
}

/**
 * aes_wrap - Wrap keys with AES Key Wrap Algorithm (128-bit KEK) (RFC3394)
 * @kek: 16-octet Key encryption key (KEK)
 * @n: Length of the plaintext key in 64-bit units; e.g., 2 = 128-bit = 16
 * bytes
 * @plain: Plaintext key to be wrapped, n * 64 bits
 * @cipher: Wrapped key, (n + 1) * 64 bits
 *@gcm_ctx:ctx
 * Returns: 0 on success, -1 on failure
 */
int aes_wrap(const unsigned char *kek, int n, const unsigned char *plain, unsigned char *cipher,aes_encrypt_ctx aes[1])
{
	unsigned char *a, *r, b[16];
	int i, j;


	a = cipher;
	r = cipher + 8;

	/* 1) Initialize variables. */
	memset(a, 0xa6, 8);
	memcpy(r, plain, 8 * n);


	/* set the AES key                          */
	aes_encrypt_key(kek, 16, aes);

	/* 2) Calculate intermediate values.
	 * For j = 0 to 5
	 *     For i=1 to n
	 *         B = AES(K, A | R[i])
	 *         A = MSB(64, B) ^ t where t = (n*j)+i
	 *         R[i] = LSB(64, B)
	 */
	for (j = 0; j <= 5; j++) {
		r = cipher + 8;
		for (i = 1; i <= n; i++) {
			memcpy(b, a, 8);
			memcpy(b + 8, r, 8);
			/* compute E(0) (for the hash function)     */
			aes_encrypt(b, b, aes);
			memcpy(a, b, 8);
			a[7] ^= n * j + i;
			memcpy(r, b + 8, 8);
			r += 8;
		}
	}
	memset(aes, 0, sizeof(aes_encrypt_ctx));

	/* 3) Output the results.
	 *
	 * These are already in @cipher due to the location of temporary
	 * variables.
	 */

	return 0;
}
int Aes_notKey_wrap_byte(unsigned char* pPlain,int iLen, unsigned char* pCipher)
{
	unsigned char bKey[16]={0x12, 0x83,0x33, 0x78, 0x12, 0x33,0x33, 0x78, 0x12, 0x63,0x33, 0x78, 0x12, 0x13,0x33, 0x78};
	aes_encrypt_ctx contx[1];
	int iNum = iLen/8;
	int nRet = aes_wrap(bKey, iNum, pPlain,  pCipher, contx);
	return nRet;
}

int aes_wrap_byte(unsigned char* pKey,unsigned char* pPlain, int iLen, unsigned char* pCipher)
{
	aes_encrypt_ctx contx[1];
	int iNum = iLen/8;
	int nRet = aes_wrap(pKey, iNum, pPlain,  pCipher, contx);
	return nRet;
}
/**
 * aes_unwrap - Unwrap key with AES Key Wrap Algorithm (128-bit KEK) (RFC3394)
 * @kek: Key encryption key (KEK)
 * @n: Length of the plaintext key in 64-bit units; e.g., 2 = 128-bit = 16
 * bytes
 * @cipher: Wrapped key to be unwrapped, (n + 1) * 64 bits
 * @plain: Plaintext key, n * 64 bits
 * Returns: 0 on success, -1 on failure (e.g., integrity verification failed)
 */
int aes_unwrap(const unsigned char *kek, int n, const unsigned char *cipher, unsigned char *plain,aes_decrypt_ctx aes[1])
{
	unsigned char a[8], *r, b[16];
	int i, j;

	/* 1) Initialize variables. */
	memcpy(a, cipher, 8);
	r = plain;
	memcpy(r, cipher + 8, 8 * n);

	aes_decrypt_key(kek, 16,aes);
	if (aes == NULL)
		return -1;

	/* 2) Compute intermediate values.
	 * For j = 5 to 0
	 *     For i = n to 1
	 *         B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i
	 *         A = MSB(64, B)
	 *         R[i] = LSB(64, B)
	 */
	for (j = 5; j >= 0; j--) {
		r = plain + (n - 1) * 8;
		for (i = n; i >= 1; i--) {
			memcpy(b, a, 8);
			b[7] ^= n * j + i;

		    memcpy(b + 8, r, 8);
			aes_decrypt(b, b, aes);
		//	aes_decrypt(ctx, b, b);
			memcpy(a, b, 8);
			memcpy(r, b + 8, 8);
			r -= 8;
		}
	}
	memset(aes, 0, sizeof(aes_decrypt_ctx));

	/* 3) Output results.
	 *
	 * These are already in @plain due to the location of temporary
	 * variables. Just verify that the IV matches with the expected value.
	 */
	for (i = 0; i < 8; i++) {
		if (a[i] != 0xa6)
			return -1;
	}

	return 0;
}

int aes_unwrap_byte(unsigned char* pKey,unsigned char* pCipher, int iLen, unsigned char* pPlain)
{
	aes_decrypt_ctx contx[1];
    int iNum = (iLen-8)/8;
	return aes_unwrap(pKey, iNum, pCipher, pPlain, contx);
}



