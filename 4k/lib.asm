; Assembly implementations of some stuff

; Data
SECTION .DATA

global _SYNC_DATA_START
global _SYNC_DATA_END
global _SYNC_DATA_SIZE

_SYNC_DATA_START:
	INCBIN "..\data_here\sync.bin"
_SYNC_DATA_END:
_SYNC_DATA_SIZE:
	dd $-_SYNC_DATA_START
	
; Text
SECTION .TEXT