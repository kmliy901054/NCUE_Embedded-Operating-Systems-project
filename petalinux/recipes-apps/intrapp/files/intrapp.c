/*
* Copyright (C) 2013 - 2016  Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define GCD_BASEADDR  0x43C00000
#define DES_BASEADDR  0x43C10000
#define AES_BASEADDR  0x43C20000
#define INTR_BASEADDR 0x43C30000
#define MAP_SIZE      0x1000

volatile uint32_t *gcd_regs;
volatile uint32_t *des_regs;
volatile uint32_t *aes_regs;
volatile uint32_t *intr_regs;

void init_ip_mapping() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/mem");
        return;
    }
    gcd_regs = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GCD_BASEADDR);
    des_regs = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, DES_BASEADDR);
    aes_regs = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, AES_BASEADDR);
    intr_regs = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, INTR_BASEADDR);
    if (gcd_regs == MAP_FAILED || des_regs == MAP_FAILED || aes_regs == MAP_FAILED || intr_regs == MAP_FAILED) {
        perror("mmap failed");
    }
    close(fd);
}

void run_gcd() {
    int x, y;
    intr_regs[0] = 2;
    printf("input two int to cal GCD：");
    scanf("%d %d", &x, &y);
    gcd_regs[0] = x;
    gcd_regs[1] = y;
    gcd_regs[2] = 1;
    gcd_regs[2] = 0;
    while (gcd_regs[4] == 0);
    int result = gcd_regs[3];
    printf("GCD(%d, %d) = %d\n", x, y, result);
}

void run_des() {
    uint64_t encryption_in, decryption_in, key;
    int mode = 0;
    intr_regs[0] = 4;
    printf("input mode(0:Encryption,1:Decryption)");
    scanf("%d", &mode);

    if(mode == 0)
    {
	printf("input Encryption input (hex，ex 8000000000000000): ");
    	scanf("%llx", &encryption_in);
    }else{
	printf("input Decryption input (hex，ex 95f8a5e5dd31d900): ");
    	scanf("%llx", &decryption_in);
    }
    
    
    
    printf("input Key (hex): ");
    scanf("%llx", &key);

    if(mode == 0)
    {
	uint32_t enc_hi = encryption_in >> 32;
    	uint32_t enc_lo = encryption_in & 0xFFFFFFFF;
	uint32_t key_hi = key >> 32;
    	uint32_t key_lo = key & 0xFFFFFFFF;
	printf("Doing Encryption...\n");
    	des_regs[0] = enc_hi;
    	des_regs[1] = enc_lo;
    	des_regs[2] = key_hi;
    	des_regs[3] = key_lo;
    	des_regs[4] = 0;  // mode: encryption
   	des_regs[5] = 1;  // start

    	while (des_regs[15] == 0);

    	uint32_t enc_out_hi = des_regs[13];
    	uint32_t enc_out_lo = des_regs[14];
    	printf("Encrypted Output: 0x%08x%08x\n", enc_out_hi, enc_out_lo);
	des_regs[5] = 0;

    }else{
	uint32_t dec_hi = decryption_in >> 32;
    	uint32_t dec_lo = decryption_in & 0xFFFFFFFF;
	uint32_t key_hi = key >> 32;
    	uint32_t key_lo = key & 0xFFFFFFFF;
	printf("Doing Decryption...\n");
	des_regs[0] = dec_hi;
	des_regs[1] = dec_lo;
	des_regs[2] = key_hi;
	des_regs[3] = key_lo;
	des_regs[4] = 1;  // mode: decryption
	des_regs[5] = 1;  // start

	while (des_regs[15] == 0);

	uint32_t dec_out_hi = des_regs[13];
	uint32_t dec_out_lo = des_regs[14];
	printf("Decrypted Output: 0x%08x%08x\n", dec_out_hi, dec_out_lo);
	des_regs[5] = 0;
    }
    
}

void run_aes() {
    uint64_t plaintext_hi, plaintext_lo, key_hi, key_lo;
    int mode = 0;
    intr_regs[0] = 8;

    printf("input mode(1:Encryption,0:Decryption)");
    scanf("%d", &mode);
    
    printf("input plaintext/encryptiontext  (hex，ex 00112233445566778899aabbccddeeff): ");
    scanf("%16llx%16llx", &plaintext_hi, &plaintext_lo);

    printf("input key  (hex，ex 000102030405060708090a0b0c0d0e0f): ");
    scanf("%16llx%16llx", &key_hi, &key_lo);
    

    if(mode == 1){
	    uint32_t P_hi = plaintext_hi >> 32;
	    uint32_t P_lo = plaintext_hi & 0xFFFFFFFF;
	    uint32_t K_hi = key_hi >> 32;
	    uint32_t K_lo = key_hi & 0xFFFFFFFF;

	    aes_regs[2] = 1;  // encryption
	    aes_regs[3] = K_hi;
	    aes_regs[4] = K_lo;
	    aes_regs[5] = P_hi;
	    aes_regs[6] = P_lo;
	    aes_regs[0] = 1;  // select hi part

	    P_hi = plaintext_lo >> 32;
	    P_lo = plaintext_lo & 0xFFFFFFFF;
	    K_hi = key_lo >> 32;
	    K_lo = key_lo & 0xFFFFFFFF;

	    aes_regs[3] = K_hi;
	    aes_regs[4] = K_lo;
	    aes_regs[5] = P_hi;
	    aes_regs[6] = P_lo;
	    aes_regs[0] = 0;  // select lo part
	    aes_regs[1] = 1;
	    aes_regs[1] = 0;

	    while (aes_regs[7] == 0);
	    uint32_t r1 = aes_regs[8], r2 = aes_regs[9], r3 = aes_regs[10], r4 = aes_regs[11];
	    printf("AES Output: 0x%08x%08x%08x%08x\n", r4, r3, r2, r1);
    }else{
	    uint32_t P_hi = plaintext_hi >> 32;
	    uint32_t P_lo = plaintext_hi & 0xFFFFFFFF;
	    uint32_t K_hi = key_hi >> 32;
	    uint32_t K_lo = key_hi & 0xFFFFFFFF;

	    aes_regs[2] = 0;  // decryption
	    aes_regs[3] = K_hi;
	    aes_regs[4] = K_lo;
	    aes_regs[5] = P_hi;
	    aes_regs[6] = P_lo;
	    aes_regs[0] = 1;  // select hi part

	    P_hi = plaintext_lo >> 32;
	    P_lo = plaintext_lo & 0xFFFFFFFF;
	    K_hi = key_lo >> 32;
	    K_lo = key_lo & 0xFFFFFFFF;

	    aes_regs[3] = K_hi;
	    aes_regs[4] = K_lo;
	    aes_regs[5] = P_hi;
	    aes_regs[6] = P_lo;
	    aes_regs[0] = 0;  // select lo part
	    aes_regs[1] = 1;
	    aes_regs[1] = 0;

	    while (aes_regs[7] == 0);
	    uint32_t r1 = aes_regs[8], r2 = aes_regs[9], r3 = aes_regs[10], r4 = aes_regs[11];
	    printf("AES Output: 0x%08x%08x%08x%08x\n", r4, r3, r2, r1);
    }
}

int main() {
    init_ip_mapping();
    while (1) {
        uint32_t mode = intr_regs[1];
        switch (mode) {
            case 1:
                run_gcd();
                break;
            case 2:
                run_des();
                break;
            case 3:
                run_aes();
                break;
            default:
		intr_regs[0] = 1;
                break;
        }
        sleep(1);
    }
    return 0;
}
