// Alex-RSA.cpp: 定义控制台应用程序的入口点。

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <malloc.h>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

/* OpenSSL headers */
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"


#include "es_log.h"

using namespace std;

#define MAX_RSA_MODULUS_BIT_LEN 		2048
#define MAX_RSA_MODULUS_LEN				(MAX_RSA_MODULUS_BIT_LEN/8)
#define MAX_RSA_PRIME_LEN				(MAX_RSA_MODULUS_BIT_LEN/(2*8))


//自定义 CSP私钥RSA(2048)
typedef struct _privatekey
{
	BLOBHEADER blobhead;
	RSAPUBKEY  rsaKey;
	BYTE	modulus[MAX_RSA_MODULUS_BIT_LEN/8];  //模数 n 
	BYTE	primeP[MAX_RSA_MODULUS_BIT_LEN /16]; // 素数p 
	BYTE	primeQ[MAX_RSA_MODULUS_BIT_LEN /16]; // 素数q
	BYTE	expDP[MAX_RSA_MODULUS_BIT_LEN /16];  // d mod ( p-1 )
	BYTE	expDQ[MAX_RSA_MODULUS_BIT_LEN /16];  // d mod ( q-1 )
	BYTE	coefficient[MAX_RSA_MODULUS_BIT_LEN /16];  //gcd(1,p)
	BYTE	expD[MAX_RSA_MODULUS_BIT_LEN /8];   //私钥d

}RSAPRIVATEKEY;

#define private_file	"E:\\source\\Alex-RSA\\RSA2048\\rsa_csp_private.dat"
#define rsa_cert		"E:\\source\\Alex-RSA\\RSA2048\\rsa_crypto.cer"
//bit-endian to litter-endian
void bytesReverses(BYTE* b,int length)
{
	vector<BYTE> t;
	t.resize(length);
	for (unsigned int i = 0; i< length; i++)
	{
		t[i] = b[i];
	}
	std::reverse(t.begin(), t.end());
	for (unsigned int i = 0; i<t.size(); i++)
	{
		b[i] = t[i];
	}
}

RSA* csptorsa(RSAPRIVATEKEY  privatekey) {

	int rv = 0;
	RSA *r;
	BIGNUM *bne, *bnn, *bnd,*bnp,*bnq,*bndp,*bndq,*bnco;
	int ret = 0;
	//构建RSA数据结构
	bne = BN_new();
	bnn = BN_new();
	bnd = BN_new();
	bnp = BN_new();
	bnq = BN_new();
	bndp = BN_new();
	bndq = BN_new();
	bnco = BN_new();

	//set rsa private value
	BN_set_word(bne, RSA_F4);
	BN_bin2bn((const unsigned char*)privatekey.modulus, MAX_RSA_MODULUS_BIT_LEN / 8, bnn);
	BN_bin2bn((const unsigned char*)privatekey.expD, MAX_RSA_MODULUS_BIT_LEN / 8, bnd);

	BN_bin2bn((const unsigned char*)privatekey.primeP, MAX_RSA_MODULUS_BIT_LEN / 16, bnp);
	BN_bin2bn((const unsigned char*)privatekey.primeQ, MAX_RSA_MODULUS_BIT_LEN / 16, bnq);
	BN_bin2bn((const unsigned char*)privatekey.expDP, MAX_RSA_MODULUS_BIT_LEN  / 16, bndp);
	BN_bin2bn((const unsigned char*)privatekey.expDQ, MAX_RSA_MODULUS_BIT_LEN  / 16, bndq);
	BN_bin2bn((const unsigned char*)privatekey.coefficient, MAX_RSA_MODULUS_BIT_LEN / 16, bnco);
	
	r = RSA_new();
	r->e = bne;
	r->d = bnd;
	r->n = bnn;
	r->p = bnp;
	r->q = bnq;
	r->dmp1 = bndp;
	r->dmq1 = bndq;
	r->iqmp = bnco;

	if ( (rv = RSA_check_key(r)) != 1)
	{
		printf("Error: %s\n", ERR_reason_error_string(ERR_get_error()));
		system("pause");
		exit(0);
	}
	return r;
	//RSA_print_fp(stdout, r, 5);
}



RSA* genrsa(int tlen)
{

	char csp_prikey[2048] = { 0 };
	int csplen = 2048;
	GetFileData(private_file, csp_prikey, &csplen);
	RSAPRIVATEKEY privatekey = { 0 };

	memcpy((unsigned char*)&privatekey,csp_prikey, csplen);
	//需要反序 
#if 1
	bytesReverses(privatekey.modulus, sizeof(privatekey.modulus));
	bytesReverses(privatekey.primeP, sizeof(privatekey.primeP));
	bytesReverses(privatekey.primeQ, sizeof(privatekey.primeQ));
	bytesReverses(privatekey.expDP, sizeof(privatekey.expDP));
	bytesReverses(privatekey.expDQ, sizeof(privatekey.expDQ));
	bytesReverses(privatekey.coefficient, sizeof(privatekey.coefficient));
	bytesReverses(privatekey.expD, sizeof(privatekey.expD));
#endif
	//得到 windows CSP标准的私钥结构privatekey

#if 0  //从文件中提取私钥测试
	string prifile("E:/RSA算法实现/RSA_PRI_PUB_KEY/nocrypt_pkcs8_rsa_private_key.pem");
	FILE *file;
	int rsa_len;
	if ((file = fopen(prifile.c_str(), "r")) == NULL) {
		return NULL;
	}

	RSA * r = PEM_read_RSAPrivateKey(file,NULL,NULL,NULL);
	return r;
#else 
	return csptorsa(privatekey);// RSA_generate_key(tlen, RSA_F4, NULL, NULL);
#endif
}

//RSA2048 ，模长2048bit=256字节  签名值=256字节
void rsa2048_sign(const char* mess1 ,RSA* rsa)
{

	unsigned char sign_value[MAX_RSA_MODULUS_BIT_LEN];                    //保存签名值的数组
	int sign_len;                                    //签名值长度
	EVP_MD_CTX mdctx;                                //摘要算法上下文变量
//	char mess1[] = "Test Message";                    //待签名的消息
	RSA *r = NULL;                                    //RSA结构体变量
	EVP_PKEY *evpKey = NULL;                            //EVP KEY结构体变量
	int i= 0 ;

	
	//rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);    //产生一个2048位的RSA密钥 ,RSA_F4 为公钥指数 
	if (rsa == NULL)
	{
		printf("gen rsa err\n");
		return;
	}

	evpKey = EVP_PKEY_new();                        //新建一个EVP_PKEY变量
	if (evpKey == NULL)
	{
		printf("EVP_PKEY_new err\n");
		RSA_free(rsa);
		return;
	}
	if (EVP_PKEY_set1_RSA(evpKey, rsa) != 1)            //保存RSA结构体到EVP_PKEY结构体
	{
		printf("EVP_PKEY_set1_RSA err\n");
		RSA_free(rsa);
		EVP_PKEY_free(evpKey);
		return;
	}
	//以下是计算签名代码
	EVP_MD_CTX_init(&mdctx);                        //初始化摘要上下文
	if (!EVP_SignInit_ex(&mdctx, EVP_sha256(), NULL))    //签名初始化，设置摘要算法，本例改为sha256
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		return;
	}
	if (!EVP_SignUpdate(&mdctx, mess1, strlen(mess1)))    //计算签名（摘要）Update
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		return;
	}
	if (!EVP_SignFinal(&mdctx, sign_value, (unsigned int*)&sign_len, evpKey))    //签名输出
	{
		printf("err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		return;
	}
	printf("消息\"%s\"的签名值是: \n", mess1);
	for (i = 0; i < sign_len; i++)
	{
		if (i % 16 == 0)
		printf("\n%08xH: ", i);
		printf("%02x ", sign_value[i]);
	}
	printf("\n");
	//SetFileData(NULL,(const char*)sign_value, sign_len);
	EVP_MD_CTX_cleanup(&mdctx);

	printf("\n正在验证签名...\n");
	//以下是验证签名代码
	EVP_MD_CTX_init(&mdctx);                            //初始化摘要上下文
	if (!EVP_VerifyInit_ex(&mdctx, EVP_sha256(), NULL))        //验证初始化，设置摘要算法，一定要和签名一致。
	{
		printf("EVP_VerifyInit_ex err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		return;
	}
	if (!EVP_VerifyUpdate(&mdctx, mess1, strlen(mess1)))    //验证签名（摘要）Update
	{
		printf("EVP_VerifyUpdate err\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		return;
	}
	if (!EVP_VerifyFinal(&mdctx, sign_value, sign_len, evpKey))//验证签名
	{
		printf("Verify err@@@\n");
		EVP_PKEY_free(evpKey);
		RSA_free(rsa);
		return;
	}
	else
	{
		printf("验证签名正确!!!!!!!!!.\n");
	}
	//释放内存
	EVP_PKEY_free(evpKey);
	RSA_free(rsa);
	EVP_MD_CTX_cleanup(&mdctx);
	CRYPTO_cleanup_all_ex_data();//这样就没有内存泄漏了。
	return;
}


int main(int arvc, char* argv[])
{	
	/* Initializing OpenSSL */
	//SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();

	rsa2048_sign("Alex-studio", genrsa(2048));
	//加解密
	cout << "\n\n" << endl;
	system("pause");
}

