.386
.MODEL FLAT, C
.CODE

CopyScreenData PROC      ; DestinationData, SourceData, BufferLength
	push ebp 
	mov  ebp, esp
	sub  esp, 20h		 ;开栈

	mov  ebx, dword ptr [ebp + 10h]	; BufferLength
	mov  esi, dword ptr [ebp + 0Ch]	; SourceData
	jmp	CopyEnd
CopyNextBlock:
							; [位置(DWORD)][不同数据的长度(DWORD)][不同的数据]
	mov edi, [ebp + 8h]		; DestinationData
	lodsd					; lodsd指令从esi指向的内存位置4个字节内容放入eax中,并且esi下移4个字节
							; 将 [位置(DWORD)] 放入eax
	add edi, eax			; DestinationData偏移eax
	lodsd					; 将 [不同数据的长度(DWORD)] 放入eax
	mov ecx, eax			; 将要拷贝的数据的长度放入ecx
	sub ebx, 8				; ebx(BufferLength)减去两个DWORD
	sub ebx, ecx			; ebx(BufferLength)减去[不同的数据]的长度
	rep movsb				; movsb搬移一个字节的数据，通过SI和DI这两个寄存器控制字符串的源地址和目标地址
							; 将DS:SI指向的地址的数据覆盖到ES:DI中,且SI和DI自动移动到下一个要搬移的地址
							; rep movsb将重复movsb,重复次数(即子串长度)存放在CX寄存器
CopyEnd:
	cmp ebx, 0				; 是否写入完 
	jnz CopyNextBlock

	add esp, 20h			;恢复堆栈
	pop ebp
	ret
CopyScreenData ENDP

END
