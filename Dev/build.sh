
cd src

ls myst.c
9c myst.c
ls myst.o
9l myst.o
mv a.out ../myst
rm myst.o
git add myst.c
git add myth.h

ls nettle.c
9c nettle.c
ls nettle.o
9l nettle.o
mv a.out ../nettle
rm nettle.o
git add nettle.c

ls lox.c
9c lox.c
ls lox.o
9l lox.o
mv a.out ../lox
rm lox.o
git add lox.c
git add lox.h

ls regs.c
9c regs.c
ls regs.o
9l regs.o
mv a.out ../regs
rm regs.o
git add regs.c

cd src-goldie
git add goldie.go
go build goldie.go
cd ..

cd ..
git add lox.asm
git add build.sh
git add run.sh
