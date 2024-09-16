
cd src

ls lox.c
9c lox.c
ls lox.o
9l lox.o
mv a.out ../lox
rm lox.o
git add lox.c
git add lox.h

cd src-goldie
ls goldie.go
git add goldie.go
go build goldie.go
cd ..

cd ..
src/src-goldie/goldie lox.asm
git add lox.asm
git add build.sh

