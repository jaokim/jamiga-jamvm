VERSION = 1
REVISION = 4

.macro DATE
.ascii "2.11.2013"
.endm

.macro VERS
.ascii "jamvm 1.4"
.endm

.macro VSTRING
.ascii "jamvm 1.4 (2.11.2013)"
.byte 13,10,0
.endm

.macro VERSTAG
.byte 0
.ascii "$VER: jamvm 1.4 (2.11.2013)"
.byte 0
.endm
