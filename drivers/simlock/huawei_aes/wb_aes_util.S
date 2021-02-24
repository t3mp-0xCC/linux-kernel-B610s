	.arch armv7-a
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"wb_aes_util.c"
	.text
.Ltext0:
	.global	__aeabi_idivmod
	.align	2
	.global	ROTL
	.type	ROTL, %function
ROTL:
	.fnstart
.LFB0:
	.file 1 "/usr1/ywx374880/B618_Master/vendor/hisi/build/release_kernel/kernel/drivers/simlock/huawei_aes/wb_aes_util.c"
	.loc 1 4 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
.LVL0:
	stmfd	sp!, {r3, r4, r5, lr}
	.save {r3, r4, r5, lr}
.LCFI0:
	.loc 1 4 0
	mov	r5, r0
	.loc 1 6 0
	mov	r0, r1
.LVL1:
	mov	r1, r2
.LVL2:
	.loc 1 4 0
	mov	r4, r2
	.loc 1 6 0
	bl	__aeabi_idivmod
.LVL3:
	.loc 1 7 0
	rsb	r3, r1, r4
	mov	r1, r5, asl r1
	orr	r5, r1, r5, asr r3
.LVL4:
	.loc 1 5 0
	mvn	r1, #0
	.loc 1 8 0
	bic	r0, r5, r1, asl r4
	.loc 1 9 0
	uxth	r0, r0
	ldmfd	sp!, {r3, r4, r5, pc}
.LFE0:
	.fnend
	.size	ROTL, .-ROTL
	.align	2
	.global	xor_block_128
	.type	xor_block_128, %function
xor_block_128:
	.fnstart
.LFB1:
	.loc 1 11 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL5:
	.loc 1 15 0
	ldr	r3, [r1, #0]
	ldr	ip, [r0, #0]
	.loc 1 16 0
	ldr	r2, [r0, #4]
	.loc 1 15 0
	eor	r3, ip, r3
	str	r3, [r0, #0]
	.loc 1 16 0
	ldr	r3, [r1, #4]
	.loc 1 17 0
	ldr	ip, [r0, #8]
	.loc 1 16 0
	eor	r3, r2, r3
	str	r3, [r0, #4]
	.loc 1 17 0
	ldr	r3, [r1, #8]
	.loc 1 18 0
	ldr	r2, [r0, #12]
	.loc 1 17 0
	eor	r3, ip, r3
	str	r3, [r0, #8]
	.loc 1 18 0
	ldr	r3, [r1, #12]
	eor	r3, r2, r3
	str	r3, [r0, #12]
	bx	lr
.LFE1:
	.fnend
	.size	xor_block_128, .-xor_block_128
	.align	2
	.global	xor_block
	.type	xor_block, %function
xor_block:
	.fnstart
.LFB2:
	.loc 1 21 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL6:
	.loc 1 23 0
	cmp	r2, #0
	.loc 1 21 0
	str	r4, [sp, #-4]!
	.save {r4}
.LCFI1:
	.loc 1 23 0
	beq	.L3
	mov	r3, #0
.LVL7:
.L4:
	.loc 1 25 0 discriminator 2
	ldrb	r4, [r1, r3]	@ zero_extendqisi2
	ldrb	ip, [r0, r3]	@ zero_extendqisi2
	eor	ip, r4, ip
	strb	ip, [r0, r3]
	.loc 1 23 0 discriminator 2
	add	r3, r3, #1
	cmp	r3, r2
	bne	.L4
.L3:
	.loc 1 27 0
	ldmfd	sp!, {r4}
	bx	lr
.LFE2:
	.fnend
	.size	xor_block, .-xor_block
	.section	.debug_frame,"",%progbits
.Lframe0:
	.4byte	.LECIE0-.LSCIE0
.LSCIE0:
	.4byte	0xffffffff
	.byte	0x1
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 -4
	.byte	0xe
	.byte	0xc
	.uleb128 0xd
	.uleb128 0
	.align	2
.LECIE0:
.LSFDE0:
	.4byte	.LEFDE0-.LASFDE0
.LASFDE0:
	.4byte	.Lframe0
	.4byte	.LFB0
	.4byte	.LFE0-.LFB0
	.byte	0x4
	.4byte	.LCFI0-.LFB0
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x4
	.byte	0x84
	.uleb128 0x3
	.byte	0x85
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x1
	.align	2
.LEFDE0:
.LSFDE2:
	.4byte	.LEFDE2-.LASFDE2
.LASFDE2:
	.4byte	.Lframe0
	.4byte	.LFB1
	.4byte	.LFE1-.LFB1
	.align	2
.LEFDE2:
.LSFDE4:
	.4byte	.LEFDE4-.LASFDE4
.LASFDE4:
	.4byte	.Lframe0
	.4byte	.LFB2
	.4byte	.LFE2-.LFB2
	.byte	0x4
	.4byte	.LCFI1-.LFB2
	.byte	0xe
	.uleb128 0x4
	.byte	0x84
	.uleb128 0x1
	.align	2
.LEFDE4:
	.text
.Letext0:
	.file 2 "/usr1/ywx374880/B618_Master/vendor/hisi/build/release_kernel/kernel/drivers/simlock/huawei_aes/wb_aes_util.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x16e
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF5
	.byte	0x1
	.4byte	.LASF6
	.4byte	.Ltext0
	.4byte	.Letext0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.ascii	"u8\000"
	.byte	0x2
	.byte	0xc
	.4byte	0x2b
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.4byte	.LASF0
	.uleb128 0x2
	.ascii	"u16\000"
	.byte	0x2
	.byte	0xd
	.4byte	0x3d
	.uleb128 0x3
	.byte	0x2
	.byte	0x7
	.4byte	.LASF1
	.uleb128 0x2
	.ascii	"u32\000"
	.byte	0x2
	.byte	0xe
	.4byte	0x4f
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.4byte	.LASF2
	.uleb128 0x3
	.byte	0x8
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x4
	.byte	0x1
	.4byte	.LASF7
	.byte	0x1
	.byte	0x3
	.byte	0x1
	.4byte	0x32
	.4byte	.LFB0
	.4byte	.LFE0
	.4byte	.LLST0
	.byte	0x1
	.4byte	0xc4
	.uleb128 0x5
	.ascii	"x\000"
	.byte	0x1
	.byte	0x3
	.4byte	0x32
	.4byte	.LLST1
	.uleb128 0x5
	.ascii	"n\000"
	.byte	0x1
	.byte	0x3
	.4byte	0xc4
	.4byte	.LLST2
	.uleb128 0x5
	.ascii	"m\000"
	.byte	0x1
	.byte	0x3
	.4byte	0xc4
	.4byte	.LLST3
	.uleb128 0x6
	.4byte	.LASF4
	.byte	0x1
	.byte	0x5
	.4byte	0x32
	.uleb128 0x7
	.ascii	"rot\000"
	.byte	0x1
	.byte	0x6
	.4byte	0xc4
	.uleb128 0x7
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x7
	.4byte	0x32
	.byte	0
	.uleb128 0x8
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x9
	.byte	0x1
	.4byte	.LASF8
	.byte	0x1
	.byte	0xa
	.byte	0x1
	.4byte	.LFB1
	.4byte	.LFE1
	.byte	0x2
	.byte	0x7d
	.sleb128 0
	.byte	0x1
	.4byte	0x111
	.uleb128 0xa
	.ascii	"tar\000"
	.byte	0x1
	.byte	0xa
	.4byte	0x111
	.byte	0x1
	.byte	0x50
	.uleb128 0xa
	.ascii	"src\000"
	.byte	0x1
	.byte	0xa
	.4byte	0x117
	.byte	0x1
	.byte	0x51
	.uleb128 0x7
	.ascii	"T\000"
	.byte	0x1
	.byte	0xc
	.4byte	0x122
	.uleb128 0x7
	.ascii	"S\000"
	.byte	0x1
	.byte	0xd
	.4byte	0x122
	.byte	0
	.uleb128 0xb
	.byte	0x4
	.4byte	0x21
	.uleb128 0xb
	.byte	0x4
	.4byte	0x11d
	.uleb128 0xc
	.4byte	0x21
	.uleb128 0xb
	.byte	0x4
	.4byte	0x44
	.uleb128 0xd
	.byte	0x1
	.4byte	.LASF9
	.byte	0x1
	.byte	0x14
	.byte	0x1
	.4byte	.LFB2
	.4byte	.LFE2
	.4byte	.LLST4
	.byte	0x1
	.uleb128 0xa
	.ascii	"tar\000"
	.byte	0x1
	.byte	0x14
	.4byte	0x111
	.byte	0x1
	.byte	0x50
	.uleb128 0xa
	.ascii	"src\000"
	.byte	0x1
	.byte	0x14
	.4byte	0x117
	.byte	0x1
	.byte	0x51
	.uleb128 0xa
	.ascii	"len\000"
	.byte	0x1
	.byte	0x14
	.4byte	0x44
	.byte	0x1
	.byte	0x52
	.uleb128 0xe
	.ascii	"i\000"
	.byte	0x1
	.byte	0x16
	.4byte	0x44
	.byte	0x1
	.byte	0x53
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2116
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2117
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LFB0-.Ltext0
	.4byte	.LCFI0-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI0-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 16
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL3-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL3-1-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LFB2-.Ltext0
	.4byte	.LCFI1-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI1-.Ltext0
	.4byte	.LFE2-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 4
	.4byte	0
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF8:
	.ascii	"xor_block_128\000"
.LASF2:
	.ascii	"unsigned int\000"
.LASF0:
	.ascii	"unsigned char\000"
.LASF3:
	.ascii	"long long unsigned int\000"
.LASF6:
	.ascii	"/usr1/ywx374880/B618_Master/vendor/hisi/build/relea"
	.ascii	"se_kernel/kernel/drivers/simlock/huawei_aes/wb_aes_"
	.ascii	"util.c\000"
.LASF9:
	.ascii	"xor_block\000"
.LASF7:
	.ascii	"ROTL\000"
.LASF1:
	.ascii	"short unsigned int\000"
.LASF4:
	.ascii	"vmask\000"
.LASF5:
	.ascii	"GNU C 4.7\000"
	.ident	"GCC: (GNU) 4.7"
