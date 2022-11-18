
.global	_cd_playtrk
.global _cd_endtrk

_cd_playtrk:
	mov	lp, r19
	movea	0x7FF, r0, r10
	not	r10, r13
	mov	r6, r11
	mov	r7, r18
	mov	r8, r16
	mov	r16, r17
	add	r10, r16 # for rounding up
	and	r13, r16
	
	add -12,sp
	movea -40,r0,r10
	st.h r0,-8[fp]
	st.b r6,-8[fp]
	st.b r10,-10[fp]
	mov -10,r6
	mov 1,r10
	st.h r0,-2[fp]
	st.b r10,-9[fp]
	add fp,r6
	mov 10,r7
	movea -128,r0,r10
	st.h r0,-6[fp]
	st.h r0,-4[fp]
	st.b r10,-1[fp]
	jal _eris_low_scsi_command
	addi	32, sp, sp
	movea	0x800, r0, r10

1:	nop
	nop
	nop
	nop
	add	-1, r10
	bne	1b

	jal	_eris_low_scsi_status

	mov	r18, r10
	mov	r19, lp
	jmp	[lp]


_cd_endtrk:
	mov	lp, r19
	movea	0x7FF, r0, r10
	not	r10, r13
	mov	r6, r11
	mov	r7, r18
	mov	r8, r16
	mov	r16, r17
	add	r10, r16 # for rounding up
	and	r13, r16
	
	add -12,sp
	movea -39,r0,r10
	st.b r7,-9[fp]
	st.b r6,-8[fp]
	st.b r10,-10[fp]
	addi -10,fp,r6
	mov 10,r7
	movea -128,r0,r10
	st.b r10,-1[fp]
	jal _eris_low_scsi_command
	addi	32, sp, sp
	movea	0x800, r0, r10

1:	nop
	nop
	nop
	nop
	add	-1, r10
	bne	1b

	jal	_eris_low_scsi_status

	mov	r18, r10
	mov	r19, lp
	jmp	[lp]
