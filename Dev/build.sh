
cd src-myst

ls myst.c
9c myst.c
ls myst.o
9l myst.o
mv a.out ../myst
rm myst.o
git add myst.c
git add myth.h
git add myth-verbose.h

cd ..
cd src-nettle

ls nettle.c
9c nettle.c
ls nettle.o
9l nettle.o
mv a.out ../nettle
rm nettle.o
git add nettle.c

cd ..
cd src-lox
ls lox.c
9c lox.c
ls lox.o
9l lox.o
mv a.out ../lox
rm lox.o
git add lox.c

cd ..
cd src-regs
ls regs.c
9c regs.c
ls regs.o
9l regs.o
mv a.out ../regs
rm regs.o
git add regs.c


cd ..
git add build.sh