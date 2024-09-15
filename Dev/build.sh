
cd src

ls myst.c
9c myst.c
ls myst.o
9l myst.o
mv a.out ../myst
rm myst.o
git add myst.c
git add myth.h

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
ls goldie.go
git add goldie.go
go build goldie.go
cd ..

cd ..
src/src-goldie/goldie lox.asm
git add lox.asm
git add build.sh
git add run.sh

