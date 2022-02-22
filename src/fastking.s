.global _king_kram_write_buffer
.global _king_kram_write_buffer_bytes

.macro	set_rrg	reg
	out.h	\reg, 0x600[r0]
.endm

.macro	set_reg reg, tmp
	movea	\reg, r0, \tmp
	set_rrg	\tmp
.endm

_king_kram_write_buffer:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	ld.w	4[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	ld.w	8[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	ld.w	12[r6],r8
	out.h	r8, 0x604[r0]
	shr 16, r8
	out.h	r8, 0x604[r0]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]


_king_kram_write_buffer_bytes:
	set_reg	0xE, r10
	add r6,r7
1:
	ld.w	0[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	ld.w	4[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	ld.w	8[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	ld.w	12[r6],r11
	mov r11,r13
	shr 8,r13
	mov r11,r15
	andi 255,r13,r14
	shl 8,r15
	andi 65280,r13,r13
	shr 24,r11
	or r15,r14
	or r11,r13
	out.h	r14, 0x604[r0]
	out.h	r13, 0x604[r0]

	addi 16,r6,r6

	cmp	r7,r6
	bl	1b
	jmp	[lp]
