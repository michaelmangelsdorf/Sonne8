/*
   Goldie
   Assembles LOX assembler source file into corestate.myst file.

   *Myth* Project
   Author: mim@ok-schalter.de (Michael/Dosflange@github)

*/

package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"log"
	"os"
	"strings"
)

/*
	 Struct for looking up opcodes for string literals
		such as numbers and mnemonics
*/
type symbol struct {
	val byte
	str string
}

var symTab = []symbol{
	{0x00, "0"},
	{0x01, "1"}, {0x02, "2"}, {0x03, "3"},
	{0x04, "4"}, {0x05, "5"}, {0x06, "6"},
	{0x07, "7"}, {0x08, "8"}, {0x09, "9"},
	{0x0A, "10"}, {0x0B, "11"}, {0x0C, "12"},
	{0x0D, "13"}, {0x0E, "14"}, {0x0F, "15"},
	{0x10, "16"}, {0x11, "17"}, {0x12, "18"},
	{0x13, "19"}, {0x14, "20"}, {0x15, "21"},
	{0x16, "22"}, {0x17, "23"}, {0x18, "24"},
	{0x19, "25"}, {0x1A, "26"}, {0x1B, "27"},
	{0x1C, "28"}, {0x1D, "29"}, {0x1E, "30"},
	{0x1F, "31"}, {0x20, "32"}, {0x21, "33"},
	{0x22, "34"}, {0x23, "35"}, {0x24, "36"},
	{0x25, "37"}, {0x26, "38"}, {0x27, "39"},
	{0x28, "40"}, {0x29, "41"}, {0x2A, "42"},
	{0x2B, "43"}, {0x2C, "44"}, {0x2D, "45"},
	{0x2E, "46"}, {0x2F, "47"}, {0x30, "48"},
	{0x31, "49"}, {0x32, "50"}, {0x33, "51"},
	{0x34, "52"}, {0x35, "53"}, {0x36, "54"},
	{0x37, "55"}, {0x38, "56"}, {0x39, "57"},
	{0x3A, "58"}, {0x3B, "59"}, {0x3C, "60"},
	{0x3D, "61"}, {0x3E, "62"}, {0x3F, "63"},
	{0x40, "64"}, {0x41, "65"}, {0x42, "66"},
	{0x43, "67"}, {0x44, "68"}, {0x45, "69"},
	{0x46, "70"}, {0x47, "71"}, {0x48, "72"},
	{0x49, "73"}, {0x4A, "74"}, {0x4B, "75"},
	{0x4C, "76"}, {0x4D, "77"}, {0x4E, "78"},
	{0x4F, "79"}, {0x50, "80"}, {0x51, "81"},
	{0x52, "82"}, {0x53, "83"}, {0x54, "84"},
	{0x55, "85"}, {0x56, "86"}, {0x57, "87"},
	{0x58, "88"}, {0x59, "89"}, {0x5A, "90"},
	{0x5B, "91"}, {0x5C, "92"}, {0x5D, "93"},
	{0x5E, "94"}, {0x5F, "95"}, {0x60, "96"},
	{0x61, "97"}, {0x62, "98"}, {0x63, "99"},
	{0x64, "100"}, {0x65, "101"}, {0x66, "102"},
	{0x67, "103"}, {0x68, "104"}, {0x69, "105"},
	{0x6A, "106"}, {0x6B, "107"}, {0x6C, "108"},
	{0x6D, "109"}, {0x6E, "110"}, {0x6F, "111"},
	{0x70, "112"}, {0x71, "113"}, {0x72, "114"},
	{0x73, "115"}, {0x74, "116"}, {0x75, "117"},
	{0x76, "118"}, {0x77, "119"}, {0x78, "120"},
	{0x79, "121"}, {0x7A, "122"}, {0x7B, "123"},
	{0x7C, "124"}, {0x7D, "125"}, {0x7E, "126"},
	{0x7F, "127"}, {0x80, "128"}, {0x81, "129"},
	{0x82, "130"}, {0x83, "131"}, {0x84, "132"},
	{0x85, "133"}, {0x86, "134"}, {0x87, "135"},
	{0x88, "136"}, {0x89, "137"}, {0x8A, "138"},
	{0x8B, "139"}, {0x8C, "140"}, {0x8D, "141"},
	{0x8E, "142"}, {0x8F, "143"}, {0x90, "144"},
	{0x91, "145"}, {0x92, "146"}, {0x93, "147"},
	{0x94, "148"}, {0x95, "149"}, {0x96, "150"},
	{0x97, "151"}, {0x98, "152"}, {0x99, "153"},
	{0x9A, "154"}, {0x9B, "155"}, {0x9C, "156"},
	{0x9D, "157"}, {0x9E, "158"}, {0x9F, "159"},
	{0xA0, "160"}, {0xA1, "161"}, {0xA2, "162"},
	{0xA3, "163"}, {0xA4, "164"}, {0xA5, "165"},
	{0xA6, "166"}, {0xA7, "167"}, {0xA8, "168"},
	{0xA9, "169"}, {0xAA, "170"}, {0xAB, "171"},
	{0xAC, "172"}, {0xAD, "173"}, {0xAE, "174"},
	{0xAF, "175"}, {0xB0, "176"}, {0xB1, "177"},
	{0xB2, "178"}, {0xB3, "179"}, {0xB4, "180"},
	{0xB5, "181"}, {0xB6, "182"}, {0xB7, "183"},
	{0xB8, "184"}, {0xB9, "185"}, {0xBA, "186"},
	{0xBB, "187"}, {0xBC, "188"}, {0xBD, "189"},
	{0xBE, "190"}, {0xBF, "191"}, {0xC0, "192"},
	{0xC1, "193"}, {0xC2, "194"}, {0xC3, "195"},
	{0xC4, "196"}, {0xC5, "197"}, {0xC6, "198"},
	{0xC7, "199"}, {0xC8, "200"}, {0xC9, "201"},
	{0xCA, "202"}, {0xCB, "203"}, {0xCC, "204"},
	{0xCD, "205"}, {0xCE, "206"}, {0xCF, "207"},
	{0xD0, "208"}, {0xD1, "209"}, {0xD2, "210"},
	{0xD3, "211"}, {0xD4, "212"}, {0xD5, "213"},
	{0xD6, "214"}, {0xD7, "215"}, {0xD8, "216"},
	{0xD9, "217"}, {0xDA, "218"}, {0xDB, "219"},
	{0xDC, "220"}, {0xDD, "221"}, {0xDE, "222"},
	{0xDF, "223"}, {0xE0, "224"}, {0xE1, "225"},
	{0xE2, "226"}, {0xE3, "227"}, {0xE4, "228"},
	{0xE5, "229"}, {0xE6, "230"}, {0xE7, "231"},
	{0xE8, "232"}, {0xE9, "233"}, {0xEA, "234"},
	{0xEB, "235"}, {0xEC, "236"}, {0xED, "237"},
	{0xEE, "238"}, {0xEF, "239"}, {0xF0, "240"},
	{0xF1, "241"}, {0xF2, "242"}, {0xF3, "243"},
	{0xF4, "244"}, {0xF5, "245"}, {0xF6, "246"},
	{0xF7, "247"}, {0xF8, "248"}, {0xF9, "249"},
	{0xFA, "250"}, {0xFB, "251"}, {0xFC, "252"},
	{0xFD, "253"}, {0xFE, "254"}, {0xFF, "255"},
	{0xFF, "-1"}, {0xFE, "-2"}, {0xFD, "-3"},
	{0xFC, "-4"}, {0xFB, "-5"}, {0xFA, "-6"},
	{0xF9, "-7"}, {0xF8, "-8"}, {0xF7, "-9"},
	{0xF6, "-10"}, {0xF5, "-11"}, {0xF4, "-12"},
	{0xF3, "-13"}, {0xF2, "-14"}, {0xF1, "-15"},
	{0xF0, "-16"}, {0xEF, "-17"}, {0xEE, "-18"},
	{0xED, "-19"}, {0xEC, "-20"}, {0xEB, "-21"},
	{0xEA, "-22"}, {0xE9, "-23"}, {0xE8, "-24"},
	{0xE7, "-25"}, {0xE6, "-26"}, {0xE5, "-27"},
	{0xE4, "-28"}, {0xE3, "-29"}, {0xE2, "-30"},
	{0xE1, "-31"}, {0xE0, "-32"}, {0xDF, "-33"},
	{0xDE, "-34"}, {0xDD, "-35"}, {0xDC, "-36"},
	{0xDB, "-37"}, {0xDA, "-38"}, {0xD9, "-39"},
	{0xD8, "-40"}, {0xD7, "-41"}, {0xD6, "-42"},
	{0xD5, "-43"}, {0xD4, "-44"}, {0xD3, "-45"},
	{0xD2, "-46"}, {0xD1, "-47"}, {0xD0, "-48"},
	{0xCF, "-49"}, {0xCE, "-50"}, {0xCD, "-51"},
	{0xCC, "-52"}, {0xCB, "-53"}, {0xCA, "-54"},
	{0xC9, "-55"}, {0xC8, "-56"}, {0xC7, "-57"},
	{0xC6, "-58"}, {0xC5, "-59"}, {0xC4, "-60"},
	{0xC3, "-61"}, {0xC2, "-62"}, {0xC1, "-63"},
	{0xC0, "-64"}, {0xBF, "-65"}, {0xBE, "-66"},
	{0xBD, "-67"}, {0xBC, "-68"}, {0xBB, "-69"},
	{0xBA, "-70"}, {0xB9, "-71"}, {0xB8, "-72"},
	{0xB7, "-73"}, {0xB6, "-74"}, {0xB5, "-75"},
	{0xB4, "-76"}, {0xB3, "-77"}, {0xB2, "-78"},
	{0xB1, "-79"}, {0xB0, "-80"}, {0xAF, "-81"},
	{0xAE, "-82"}, {0xAD, "-83"}, {0xAC, "-84"},
	{0xAB, "-85"}, {0xAA, "-86"}, {0xA9, "-87"},
	{0xA8, "-88"}, {0xA7, "-89"}, {0xA6, "-90"},
	{0xA5, "-91"}, {0xA4, "-92"}, {0xA3, "-93"},
	{0xA2, "-94"}, {0xA1, "-95"}, {0xA0, "-96"},
	{0x9F, "-97"}, {0x9E, "-98"}, {0x9D, "-99"},
	{0x9C, "-100"}, {0x9B, "-101"}, {0x9A, "-102"},
	{0x99, "-103"}, {0x98, "-104"}, {0x97, "-105"},
	{0x96, "-106"}, {0x95, "-107"}, {0x94, "-108"},
	{0x93, "-109"}, {0x92, "-110"}, {0x91, "-111"},
	{0x90, "-112"}, {0x8F, "-113"}, {0x8E, "-114"},
	{0x8D, "-115"}, {0x8C, "-116"}, {0x8B, "-117"},
	{0x8A, "-118"}, {0x89, "-119"}, {0x88, "-120"},
	{0x87, "-121"}, {0x86, "-122"}, {0x85, "-123"},
	{0x84, "-124"}, {0x83, "-125"}, {0x82, "-126"},
	{0x81, "-127"}, {0x80, "-128"}, {0x00, "00h"},
	{0x01, "01h"}, {0x02, "02h"}, {0x03, "03h"},
	{0x04, "04h"}, {0x05, "05h"}, {0x06, "06h"},
	{0x07, "07h"}, {0x08, "08h"}, {0x09, "09h"},
	{0x0A, "0Ah"}, {0x0B, "0Bh"}, {0x0C, "0Ch"},
	{0x0D, "0Dh"}, {0x0E, "0Eh"}, {0x0F, "0Fh"},
	{0x10, "10h"}, {0x11, "11h"}, {0x12, "12h"},
	{0x13, "13h"}, {0x14, "14h"}, {0x15, "15h"},
	{0x16, "16h"}, {0x17, "17h"}, {0x18, "18h"},
	{0x19, "19h"}, {0x1A, "1Ah"}, {0x1B, "1Bh"},
	{0x1C, "1Ch"}, {0x1D, "1Dh"}, {0x1E, "1Eh"},
	{0x1F, "1Fh"}, {0x20, "20h"}, {0x21, "21h"},
	{0x22, "22h"}, {0x23, "23h"}, {0x24, "24h"},
	{0x25, "25h"}, {0x26, "26h"}, {0x27, "27h"},
	{0x28, "28h"}, {0x29, "29h"}, {0x2A, "2Ah"},
	{0x2B, "2Bh"}, {0x2C, "2Ch"}, {0x2D, "2Dh"},
	{0x2E, "2Eh"}, {0x2F, "2Fh"}, {0x30, "30h"},
	{0x31, "31h"}, {0x32, "32h"}, {0x33, "33h"},
	{0x34, "34h"}, {0x35, "35h"}, {0x36, "36h"},
	{0x37, "37h"}, {0x38, "38h"}, {0x39, "39h"},
	{0x3A, "3Ah"}, {0x3B, "3Bh"}, {0x3C, "3Ch"},
	{0x3D, "3Dh"}, {0x3E, "3Eh"}, {0x3F, "3Fh"},
	{0x40, "40h"}, {0x41, "41h"}, {0x42, "42h"},
	{0x43, "43h"}, {0x44, "44h"}, {0x45, "45h"},
	{0x46, "46h"}, {0x47, "47h"}, {0x48, "48h"},
	{0x49, "49h"}, {0x4A, "4Ah"}, {0x4B, "4Bh"},
	{0x4C, "4Ch"}, {0x4D, "4Dh"}, {0x4E, "4Eh"},
	{0x4F, "4Fh"}, {0x50, "50h"}, {0x51, "51h"},
	{0x52, "52h"}, {0x53, "53h"}, {0x54, "54h"},
	{0x55, "55h"}, {0x56, "56h"}, {0x57, "57h"},
	{0x58, "58h"}, {0x59, "59h"}, {0x5A, "5Ah"},
	{0x5B, "5Bh"}, {0x5C, "5Ch"}, {0x5D, "5Dh"},
	{0x5E, "5Eh"}, {0x5F, "5Fh"}, {0x60, "60h"},
	{0x61, "61h"}, {0x62, "62h"}, {0x63, "63h"},
	{0x64, "64h"}, {0x65, "65h"}, {0x66, "66h"},
	{0x67, "67h"}, {0x68, "68h"}, {0x69, "69h"},
	{0x6A, "6Ah"}, {0x6B, "6Bh"}, {0x6C, "6Ch"},
	{0x6D, "6Dh"}, {0x6E, "6Eh"}, {0x6F, "6Fh"},
	{0x70, "70h"}, {0x71, "71h"}, {0x72, "72h"},
	{0x73, "73h"}, {0x74, "74h"}, {0x75, "75h"},
	{0x76, "76h"}, {0x77, "77h"}, {0x78, "78h"},
	{0x79, "79h"}, {0x7A, "7Ah"}, {0x7B, "7Bh"},
	{0x7C, "7Ch"}, {0x7D, "7Dh"}, {0x7E, "7Eh"},
	{0x7F, "7Fh"}, {0x80, "80h"}, {0x81, "81h"},
	{0x82, "82h"}, {0x83, "83h"}, {0x84, "84h"},
	{0x85, "85h"}, {0x86, "86h"}, {0x87, "87h"},
	{0x88, "88h"}, {0x89, "89h"}, {0x8A, "8Ah"},
	{0x8B, "8Bh"}, {0x8C, "8Ch"}, {0x8D, "8Dh"},
	{0x8E, "8Eh"}, {0x8F, "8Fh"}, {0x90, "90h"},
	{0x91, "91h"}, {0x92, "92h"}, {0x93, "93h"},
	{0x94, "94h"}, {0x95, "95h"}, {0x96, "96h"},
	{0x97, "97h"}, {0x98, "98h"}, {0x99, "99h"},
	{0x9A, "9Ah"}, {0x9B, "9Bh"}, {0x9C, "9Ch"},
	{0x9D, "9Dh"}, {0x9E, "9Eh"}, {0x9F, "9Fh"},
	{0xA0, "A0h"}, {0xA1, "A1h"}, {0xA2, "A2h"},
	{0xA3, "A3h"}, {0xA4, "A4h"}, {0xA5, "A5h"},
	{0xA6, "A6h"}, {0xA7, "A7h"}, {0xA8, "A8h"},
	{0xA9, "A9h"}, {0xAA, "AAh"}, {0xAB, "ABh"},
	{0xAC, "ACh"}, {0xAD, "ADh"}, {0xAE, "AEh"},
	{0xAF, "AFh"}, {0xB0, "B0h"}, {0xB1, "B1h"},
	{0xB2, "B2h"}, {0xB3, "B3h"}, {0xB4, "B4h"},
	{0xB5, "B5h"}, {0xB6, "B6h"}, {0xB7, "B7h"},
	{0xB8, "B8h"}, {0xB9, "B9h"}, {0xBA, "BAh"},
	{0xBB, "BBh"}, {0xBC, "BCh"}, {0xBD, "BDh"},
	{0xBE, "BEh"}, {0xBF, "BFh"}, {0xC0, "C0h"},
	{0xC1, "C1h"}, {0xC2, "C2h"}, {0xC3, "C3h"},
	{0xC4, "C4h"}, {0xC5, "C5h"}, {0xC6, "C6h"},
	{0xC7, "C7h"}, {0xC8, "C8h"}, {0xC9, "C9h"},
	{0xCA, "CAh"}, {0xCB, "CBh"}, {0xCC, "CCh"},
	{0xCD, "CDh"}, {0xCE, "CEh"}, {0xCF, "CFh"},
	{0xD0, "D0h"}, {0xD1, "D1h"}, {0xD2, "D2h"},
	{0xD3, "D3h"}, {0xD4, "D4h"}, {0xD5, "D5h"},
	{0xD6, "D6h"}, {0xD7, "D7h"}, {0xD8, "D8h"},
	{0xD9, "D9h"}, {0xDA, "DAh"}, {0xDB, "DBh"},
	{0xDC, "DCh"}, {0xDD, "DDh"}, {0xDE, "DEh"},
	{0xDF, "DFh"}, {0xE0, "E0h"}, {0xE1, "E1h"},
	{0xE2, "E2h"}, {0xE3, "E3h"}, {0xE4, "E4h"},
	{0xE5, "E5h"}, {0xE6, "E6h"}, {0xE7, "E7h"},
	{0xE8, "E8h"}, {0xE9, "E9h"}, {0xEA, "EAh"},
	{0xEB, "EBh"}, {0xEC, "ECh"}, {0xED, "EDh"},
	{0xEE, "EEh"}, {0xEF, "EFh"}, {0xF0, "F0h"},
	{0xF1, "F1h"}, {0xF2, "F2h"}, {0xF3, "F3h"},
	{0xF4, "F4h"}, {0xF5, "F5h"}, {0xF6, "F6h"},
	{0xF7, "F7h"}, {0xF8, "F8h"}, {0xF9, "F9h"},
	{0xFA, "FAh"}, {0xFB, "FBh"}, {0xFC, "FCh"},
	{0xFD, "FDh"}, {0xFE, "FEh"}, {0xFF, "FFh"},
	{0x00, "0h"}, {0x01, "1h"}, {0x02, "2h"},
	{0x03, "3h"}, {0x04, "4h"}, {0x05, "5h"},
	{0x06, "6h"}, {0x07, "7h"}, {0x08, "8h"},
	{0x09, "9h"}, {0x0A, "Ah"}, {0x0B, "Bh"},
	{0x0C, "Ch"}, {0x0D, "Dh"}, {0x0E, "Eh"},
	{0x0F, "Fh"}, {0x00, "0000_0000b"}, {0x01, "0000_0001b"},
	{0x02, "0000_0010b"}, {0x03, "0000_0011b"}, {0x04, "0000_0100b"},
	{0x05, "0000_0101b"}, {0x06, "0000_0110b"}, {0x07, "0000_0111b"},
	{0x08, "0000_1000b"}, {0x09, "0000_1001b"}, {0x0A, "0000_1010b"},
	{0x0B, "0000_1011b"}, {0x0C, "0000_1100b"}, {0x0D, "0000_1101b"},
	{0x0E, "0000_1110b"}, {0x0F, "0000_1111b"}, {0x10, "0001_0000b"},
	{0x11, "0001_0001b"}, {0x12, "0001_0010b"}, {0x13, "0001_0011b"},
	{0x14, "0001_0100b"}, {0x15, "0001_0101b"}, {0x16, "0001_0110b"},
	{0x17, "0001_0111b"}, {0x18, "0001_1000b"}, {0x19, "0001_1001b"},
	{0x1A, "0001_1010b"}, {0x1B, "0001_1011b"}, {0x1C, "0001_1100b"},
	{0x1D, "0001_1101b"}, {0x1E, "0001_1110b"}, {0x1F, "0001_1111b"},
	{0x20, "0010_0000b"}, {0x21, "0010_0001b"}, {0x22, "0010_0010b"},
	{0x23, "0010_0011b"}, {0x24, "0010_0100b"}, {0x25, "0010_0101b"},
	{0x26, "0010_0110b"}, {0x27, "0010_0111b"}, {0x28, "0010_1000b"},
	{0x29, "0010_1001b"}, {0x2A, "0010_1010b"}, {0x2B, "0010_1011b"},
	{0x2C, "0010_1100b"}, {0x2D, "0010_1101b"}, {0x2E, "0010_1110b"},
	{0x2F, "0010_1111b"}, {0x30, "0011_0000b"}, {0x31, "0011_0001b"},
	{0x32, "0011_0010b"}, {0x33, "0011_0011b"}, {0x34, "0011_0100b"},
	{0x35, "0011_0101b"}, {0x36, "0011_0110b"}, {0x37, "0011_0111b"},
	{0x38, "0011_1000b"}, {0x39, "0011_1001b"}, {0x3A, "0011_1010b"},
	{0x3B, "0011_1011b"}, {0x3C, "0011_1100b"}, {0x3D, "0011_1101b"},
	{0x3E, "0011_1110b"}, {0x3F, "0011_1111b"}, {0x40, "0100_0000b"},
	{0x41, "0100_0001b"}, {0x42, "0100_0010b"}, {0x43, "0100_0011b"},
	{0x44, "0100_0100b"}, {0x45, "0100_0101b"}, {0x46, "0100_0110b"},
	{0x47, "0100_0111b"}, {0x48, "0100_1000b"}, {0x49, "0100_1001b"},
	{0x4A, "0100_1010b"}, {0x4B, "0100_1011b"}, {0x4C, "0100_1100b"},
	{0x4D, "0100_1101b"}, {0x4E, "0100_1110b"}, {0x4F, "0100_1111b"},
	{0x50, "0101_0000b"}, {0x51, "0101_0001b"}, {0x52, "0101_0010b"},
	{0x53, "0101_0011b"}, {0x54, "0101_0100b"}, {0x55, "0101_0101b"},
	{0x56, "0101_0110b"}, {0x57, "0101_0111b"}, {0x58, "0101_1000b"},
	{0x59, "0101_1001b"}, {0x5A, "0101_1010b"}, {0x5B, "0101_1011b"},
	{0x5C, "0101_1100b"}, {0x5D, "0101_1101b"}, {0x5E, "0101_1110b"},
	{0x5F, "0101_1111b"}, {0x60, "0110_0000b"}, {0x61, "0110_0001b"},
	{0x62, "0110_0010b"}, {0x63, "0110_0011b"}, {0x64, "0110_0100b"},
	{0x65, "0110_0101b"}, {0x66, "0110_0110b"}, {0x67, "0110_0111b"},
	{0x68, "0110_1000b"}, {0x69, "0110_1001b"}, {0x6A, "0110_1010b"},
	{0x6B, "0110_1011b"}, {0x6C, "0110_1100b"}, {0x6D, "0110_1101b"},
	{0x6E, "0110_1110b"}, {0x6F, "0110_1111b"}, {0x70, "0111_0000b"},
	{0x71, "0111_0001b"}, {0x72, "0111_0010b"}, {0x73, "0111_0011b"},
	{0x74, "0111_0100b"}, {0x75, "0111_0101b"}, {0x76, "0111_0110b"},
	{0x77, "0111_0111b"}, {0x78, "0111_1000b"}, {0x79, "0111_1001b"},
	{0x7A, "0111_1010b"}, {0x7B, "0111_1011b"}, {0x7C, "0111_1100b"},
	{0x7D, "0111_1101b"}, {0x7E, "0111_1110b"}, {0x7F, "0111_1111b"},
	{0x80, "1000_0000b"}, {0x81, "1000_0001b"}, {0x82, "1000_0010b"},
	{0x83, "1000_0011b"}, {0x84, "1000_0100b"}, {0x85, "1000_0101b"},
	{0x86, "1000_0110b"}, {0x87, "1000_0111b"}, {0x88, "1000_1000b"},
	{0x89, "1000_1001b"}, {0x8A, "1000_1010b"}, {0x8B, "1000_1011b"},
	{0x8C, "1000_1100b"}, {0x8D, "1000_1101b"}, {0x8E, "1000_1110b"},
	{0x8F, "1000_1111b"}, {0x90, "1001_0000b"}, {0x91, "1001_0001b"},
	{0x92, "1001_0010b"}, {0x93, "1001_0011b"}, {0x94, "1001_0100b"},
	{0x95, "1001_0101b"}, {0x96, "1001_0110b"}, {0x97, "1001_0111b"},
	{0x98, "1001_1000b"}, {0x99, "1001_1001b"}, {0x9A, "1001_1010b"},
	{0x9B, "1001_1011b"}, {0x9C, "1001_1100b"}, {0x9D, "1001_1101b"},
	{0x9E, "1001_1110b"}, {0x9F, "1001_1111b"}, {0xA0, "1010_0000b"},
	{0xA1, "1010_0001b"}, {0xA2, "1010_0010b"}, {0xA3, "1010_0011b"},
	{0xA4, "1010_0100b"}, {0xA5, "1010_0101b"}, {0xA6, "1010_0110b"},
	{0xA7, "1010_0111b"}, {0xA8, "1010_1000b"}, {0xA9, "1010_1001b"},
	{0xAA, "1010_1010b"}, {0xAB, "1010_1011b"}, {0xAC, "1010_1100b"},
	{0xAD, "1010_1101b"}, {0xAE, "1010_1110b"}, {0xAF, "1010_1111b"},
	{0xB0, "1011_0000b"}, {0xB1, "1011_0001b"}, {0xB2, "1011_0010b"},
	{0xB3, "1011_0011b"}, {0xB4, "1011_0100b"}, {0xB5, "1011_0101b"},
	{0xB6, "1011_0110b"}, {0xB7, "1011_0111b"}, {0xB8, "1011_1000b"},
	{0xB9, "1011_1001b"}, {0xBA, "1011_1010b"}, {0xBB, "1011_1011b"},
	{0xBC, "1011_1100b"}, {0xBD, "1011_1101b"}, {0xBE, "1011_1110b"},
	{0xBF, "1011_1111b"}, {0xC0, "1100_0000b"}, {0xC1, "1100_0001b"},
	{0xC2, "1100_0010b"}, {0xC3, "1100_0011b"}, {0xC4, "1100_0100b"},
	{0xC5, "1100_0101b"}, {0xC6, "1100_0110b"}, {0xC7, "1100_0111b"},
	{0xC8, "1100_1000b"}, {0xC9, "1100_1001b"}, {0xCA, "1100_1010b"},
	{0xCB, "1100_1011b"}, {0xCC, "1100_1100b"}, {0xCD, "1100_1101b"},
	{0xCE, "1100_1110b"}, {0xCF, "1100_1111b"}, {0xD0, "1101_0000b"},
	{0xD1, "1101_0001b"}, {0xD2, "1101_0010b"}, {0xD3, "1101_0011b"},
	{0xD4, "1101_0100b"}, {0xD5, "1101_0101b"}, {0xD6, "1101_0110b"},
	{0xD7, "1101_0111b"}, {0xD8, "1101_1000b"}, {0xD9, "1101_1001b"},
	{0xDA, "1101_1010b"}, {0xDB, "1101_1011b"}, {0xDC, "1101_1100b"},
	{0xDD, "1101_1101b"}, {0xDE, "1101_1110b"}, {0xDF, "1101_1111b"},
	{0xE0, "1110_0000b"}, {0xE1, "1110_0001b"}, {0xE2, "1110_0010b"},
	{0xE3, "1110_0011b"}, {0xE4, "1110_0100b"}, {0xE5, "1110_0101b"},
	{0xE6, "1110_0110b"}, {0xE7, "1110_0111b"}, {0xE8, "1110_1000b"},
	{0xE9, "1110_1001b"}, {0xEA, "1110_1010b"}, {0xEB, "1110_1011b"},
	{0xEC, "1110_1100b"}, {0xED, "1110_1101b"}, {0xEE, "1110_1110b"},
	{0xEF, "1110_1111b"}, {0xF0, "1111_0000b"}, {0xF1, "1111_0001b"},
	{0xF2, "1111_0010b"}, {0xF3, "1111_0011b"}, {0xF4, "1111_0100b"},
	{0xF5, "1111_0101b"}, {0xF6, "1111_0110b"}, {0xF7, "1111_0111b"},
	{0xF8, "1111_1000b"}, {0xF9, "1111_1001b"}, {0xFA, "1111_1010b"},
	{0xFB, "1111_1011b"}, {0xFC, "1111_1100b"}, {0xFD, "1111_1101b"},
	{0xFE, "1111_1110b"}, {0xFF, "1111_1111b"}, {0x00, "0000b"},
	{0x01, "0001b"}, {0x02, "0010b"}, {0x03, "0011b"},
	{0x04, "0100b"}, {0x05, "0101b"}, {0x06, "0110b"},
	{0x07, "0111b"}, {0x08, "1000b"}, {0x09, "1001b"},
	{0x0A, "1010b"}, {0x0B, "1011b"}, {0x0C, "1100b"},
	{0x0D, "1101b"}, {0x0E, "1110b"}, {0x0F, "1111b"},
	{0x20, "'SP'"}, {0x21, "'!'"}, {0x22, "'\"'"},
	{0x23, "'#'"}, {0x24, "'$'"}, {0x25, "'%'"},
	{0x26, "'&'"}, {0x27, "'''"}, {0x28, "'('"},
	{0x29, "')'"}, {0x2A, "'*'"}, {0x2B, "'+'"},
	{0x2C, "','"}, {0x2D, "'-'"}, {0x2E, "'.'"},
	{0x2F, "'/'"}, {0x30, "'0'"}, {0x31, "'1'"},
	{0x32, "'2'"}, {0x33, "'3'"}, {0x34, "'4'"},
	{0x35, "'5'"}, {0x36, "'6'"}, {0x37, "'7'"},
	{0x38, "'8'"}, {0x39, "'9'"}, {0x3A, "':'"},
	{0x3B, "';'"}, {0x3C, "'<'"}, {0x3D, "'='"},
	{0x3E, "'>'"}, {0x3F, "'?'"}, {0x40, "'@'"},
	{0x41, "'A'"}, {0x42, "'B'"}, {0x43, "'C'"},
	{0x44, "'D'"}, {0x45, "'E'"}, {0x46, "'F'"},
	{0x47, "'G'"}, {0x48, "'H'"}, {0x49, "'I'"},
	{0x4A, "'J'"}, {0x4B, "'K'"}, {0x4C, "'L'"},
	{0x4D, "'M'"}, {0x4E, "'N'"}, {0x4F, "'O'"},
	{0x50, "'P'"}, {0x51, "'Q'"}, {0x52, "'R'"},
	{0x53, "'S'"}, {0x54, "'T'"}, {0x55, "'U'"},
	{0x56, "'V'"}, {0x57, "'W'"}, {0x58, "'X'"},
	{0x59, "'Y'"}, {0x5A, "'Z'"}, {0x5B, "'['"},
	{0x5C, "'\\'"}, {0x5D, "']'"}, {0x5E, "'^'"},
	{0x5F, "'_'"}, {0x60, "'`'"}, {0x61, "'a'"},
	{0x62, "'b'"}, {0x63, "'c'"}, {0x64, "'d'"},
	{0x65, "'e'"}, {0x66, "'f'"}, {0x67, "'g'"},
	{0x68, "'h'"}, {0x69, "'i'"}, {0x6A, "'j'"},
	{0x6B, "'k'"}, {0x6C, "'l'"}, {0x6D, "'m'"},
	{0x6E, "'n'"}, {0x6F, "'o'"}, {0x70, "'p'"},
	{0x71, "'q'"}, {0x72, "'r'"}, {0x73, "'s'"},
	{0x74, "'t'"}, {0x75, "'u'"}, {0x76, "'v'"},
	{0x77, "'w'"}, {0x78, "'x'"}, {0x79, "'y'"},
	{0x7A, "'z'"}, {0x7B, "'{'"}, {0x7C, "'|'"},
	{0x7D, "'}'"}, {0x7E, "'~'"}, {0x00, "'NUL'"},
	{0x0D, "'CR'"}, {0x0A, "'LF'"},
	/*SYS*/
	{0x00, "NOP"},
	{0x01, "SSI"}, {0x02, "SSO"}, {0x03, "SCL"}, {0x04, "SCH"},
	{0x05, "RET"}, {0x06, "COR"}, {0x07, "OWN"},
	/*FIX*/
	{0x08, "P4"},
	{0x09, "P1"}, {0x0A, "P2"}, {0x0B, "P3"}, {0x0C, "M4"},
	{0x0D, "M3"}, {0x0E, "M2"}, {0x0F, "M1"},
	/*ALU*/
	{0x10, "CLR"},
	{0x11, "IDO"}, {0x12, "OCR"}, {0x13, "OCO"}, {0x14, "SLR"},
	{0x15, "SLO"}, {0x16, "SRR"}, {0x17, "SRO"}, {0x18, "AND"},
	{0x19, "IOR"}, {0x1A, "EOR"}, {0x1B, "ADD"}, {0x1C, "CAR"},
	{0x1D, "RLO"}, {0x1E, "REO"}, {0x1F, "RGO"},
	/*TRAP*/
	{0x20, "*0"},
	{0x21, "*1"}, {0x22, "*2"}, {0x23, "*3"}, {0x24, "*4"},
	{0x25, "*5"}, {0x26, "*6"}, {0x27, "*7"}, {0x28, "*8"},
	{0x29, "*9"}, {0x2A, "*10"}, {0x2B, "*11"}, {0x2C, "*12"},
	{0x2D, "*13"}, {0x2E, "*14"}, {0x2F, "*15"}, {0x30, "*16"},
	{0x31, "*17"}, {0x32, "*18"}, {0x33, "*19"}, {0x34, "*20"},
	{0x35, "*21"}, {0x36, "*22"}, {0x37, "*23"}, {0x38, "*24"},
	{0x39, "*25"}, {0x3A, "*26"}, {0x3B, "*27"}, {0x3C, "*28"},
	{0x3D, "*29"}, {0x3E, "*30"}, {0x3F, "*31"},
	/*GIRO*/
	{0x40, "0g"}, {0x41, "1g"}, {0x42, "2g"}, {0x43, "3g"}, {0x44, "4g"}, {0x45, "5g"}, {0x46, "6g"}, {0x47, "7g"},
	{0x48, "g0"}, {0x49, "g1"}, {0x4A, "g2"}, {0x4B, "g3"}, {0x4C, "g4"}, {0x4D, "g5"}, {0x4E, "g6"}, {0x4F, "g7"},
	{0x50, "0i"}, {0x51, "1i"}, {0x52, "2i"}, {0x53, "3i"}, {0x54, "4i"}, {0x55, "5i"}, {0x56, "6i"}, {0x57, "7i"},
	{0x58, "i0"}, {0x59, "i1"}, {0x5A, "i2"}, {0x5B, "i3"}, {0x5C, "i4"}, {0x5D, "i5"}, {0x5E, "i6"}, {0x5F, "i7"},
	{0x60, "0r"}, {0x61, "1r"}, {0x62, "2r"}, {0x63, "3r"}, {0x64, "4r"}, {0x65, "5r"}, {0x66, "6r"}, {0x67, "7r"},
	{0x68, "r0"}, {0x69, "r1"}, {0x6A, "r2"}, {0x6B, "r3"}, {0x6C, "r4"}, {0x6D, "r5"}, {0x6E, "r6"}, {0x6F, "r7"},
	{0x70, "0o"}, {0x71, "1o"}, {0x72, "2o"}, {0x73, "3o"}, {0x74, "4o"}, {0x75, "5o"}, {0x76, "6o"}, {0x77, "7o"},
	{0x78, "o0"}, {0x79, "o1"}, {0x7A, "o2"}, {0x7B, "o3"}, {0x7C, "o4"}, {0x7D, "o5"}, {0x7E, "o6"}, {0x7F, "o7"},
	/*PAIR*/
	{0x80, "no"}, {0x81, "END"}, {0x82, "SCROUNGE_NL"}, {0x83, "ng"}, {0x84, "nr"}, {0x85, "ni"}, {0x86, "ns"}, {0x87, "np"}, {0x88, "ne"}, {0x89, "na"}, {0x8A, "nb"}, {0x8B, "nj"}, {0x8C, "nw"}, {0x8D, "nt"}, {0x8E, "nf"}, {0x8F, "nc"},
	{0x90, "mo"}, {0x91, "SCROUNGE_MM"}, {0x92, "SCROUNGE_ML"}, {0x93, "mg"}, {0x94, "mr"}, {0x95, "mi"}, {0x96, "ms"}, {0x97, "mp"}, {0x98, "me"}, {0x99, "ma"}, {0x9A, "mb"}, {0x9B, "mj"}, {0x9C, "mw"}, {0x9D, "mt"}, {0x9E, "mf"}, {0x9F, "mc"},
	{0xA0, "lo"}, {0xA1, "SCROUNGE_LM"}, {0xA2, "SCROUNGE_LL"}, {0xA3, "lg"}, {0xA4, "lr"}, {0xA5, "li"}, {0xA6, "ls"}, {0xA7, "lp"}, {0xA8, "le"}, {0xA9, "la"}, {0xAA, "lb"}, {0xAB, "lj"}, {0xAC, "lw"}, {0xAD, "lt"}, {0xAE, "lf"}, {0xAF, "lc"},
	{0xB0, "go"}, {0xB1, "gm"}, {0xB2, "gl"}, {0xB3, "SCROUNGE_GG"}, {0xB4, "gr"}, {0xB5, "gi"}, {0xB6, "gs"}, {0xB7, "gp"}, {0xB8, "ge"}, {0xB9, "ga"}, {0xBA, "gb"}, {0xBB, "gj"}, {0xBC, "gw"}, {0xBD, "gt"}, {0xBE, "gf"}, {0xBF, "gc"},
	{0xC0, "ro"}, {0xC1, "rm"}, {0xC2, "rl"}, {0xC3, "rg"}, {0xC4, "SCROUNGE_RR"}, {0xC5, "ri"}, {0xC6, "rs"}, {0xC7, "rp"}, {0xC8, "re"}, {0xC9, "ra"}, {0xCA, "rb"}, {0xCB, "rj"}, {0xCC, "rw"}, {0xCD, "rt"}, {0xCE, "rf"}, {0xCF, "rc"},
	{0xD0, "io"}, {0xD1, "im"}, {0xD2, "il"}, {0xD3, "ig"}, {0xD4, "ir"}, {0xD5, "SCROUNGE_II"}, {0xD6, "is"}, {0xD7, "ip"}, {0xD8, "ie"}, {0xD9, "ia"}, {0xDA, "ib"}, {0xDB, "ij"}, {0xDC, "iw"}, {0xDD, "it"}, {0xDE, "if"}, {0xDF, "ic"},
	{0xE0, "so"}, {0xE1, "sm"}, {0xE2, "sl"}, {0xE3, "sg"}, {0xE4, "sr"}, {0xE5, "si"}, {0xE6, "ss"}, {0xE7, "sp"}, {0xE8, "se"}, {0xE9, "sa"}, {0xEA, "sb"}, {0xEB, "sj"}, {0xEC, "sw"}, {0xED, "st"}, {0xEE, "sf"}, {0xEF, "sc"},
	{0xF0, "po"}, {0xF1, "pm"}, {0xF2, "pl"}, {0xF3, "pg"}, {0xF4, "pr"}, {0xF5, "pi"}, {0xF6, "ps"}, {0xF7, "pp"}, {0xF8, "pe"}, {0xF9, "pa"}, {0xFA, "pb"}, {0xFB, "pj"}, {0xFC, "pw"}, {0xFD, "pt"}, {0xFE, "pf"}, {0xFF, "pc"},
}

type myth_vm struct /*Complete machine state including all ram*/
{
	ram [256][256]byte /*MemoryByte[page][offset]*/

	irq  byte /*Interrupt request bit - set by PERIPHERY*/

	e_old byte; /*Device ENABLE register previous value - set by VM */
	e_new byte; /*Device ENABLE register current value - set by VM */

	sclk byte /*Serial clock state bit - set by VM*/
	miso byte /*Serial input line state bit - set by PERIPHERY*/
	mosi byte /*Serial output line state bit - set by VM*/
	sir byte /*Serial input register - set by VM*/
	sor byte /*Serial output register - set by VM*/

	pir byte /*Parallel input register - set by PERIPHERY*/
	por byte /*Parallel output register - set by VM*/

	r byte /*Result*/
	o byte /*Operand*/

	i  byte /*Inner Counter*/
	pc byte /*Program Counter*/

	d  byte /*Coroutine page index*/
	c  byte /*Code page index*/
	g  byte /*Data page index*/
	l  byte /*Local page index*/

	scrounge byte /*Set by VM if scrounge opcode executed, else zero*/
}

var vm = myth_vm{}
var srcLine []string
var pageLabel [256]string
var offsLabel [256][256]string
var defConst = [1024]symbol{} // Defined constants
var blameLine [256][256]int   // Source line that generated this object byte
var page byte = 0             // Global used during assembly, current page
var offs byte = 0             // Current offset
var lineNum = 1               // Current line of source code
var insideComment = false     // Character position is inside a comment
var insideString = false      // Character position is inside a string literal
var chPos = 0                 // Character position in source line
var constTopIndex = 0         // Insertion point into defConst array
var lid [256]byte             // Last inserted object byte in the page

func ldSrc(fname string) []byte {
	var e error
	var t []byte
	t, e = os.ReadFile(fname)
	if e != nil {
		fmt.Printf("Missing input file %s\n", fname)
		os.Exit(1)
	}
	return t
}

func wrDebugTxt() {

	f, e := os.Create("lox_debug.txt")
	if e != nil {
		log.Fatal("Could not create output file")
	}
	defer f.Close()

	w := bufio.NewWriter(f)

	fmt.Fprintf(w, "Goldie/LOX assembly log\n")
	fmt.Fprintf(w, "Frame.Offset     Object code     Lid Line# Source text\n\n")

	var fill int
	var i byte
	var j byte
	var pos int
	var firstLine bool
	var overline bool
	var lid0 byte

	for line := 1; line <= len(srcLine); line++ {
		//fmt.Fprintf(w, "---------------------------------------\n")

		fill = 0
		pos = 0
		overline = false
		if line == blameLine[i][j] {
			lid0 = lid[i]
			fmt.Fprintf(w, "%.02X.%.02X:  ", i, j)

			fill = 8
			firstLine = true
			for line == blameLine[i][j] {
				if (pos%8) == 0 && !firstLine {
					fmt.Fprintf(w, "\n        ")
					fill = 8
					overline = true
				}
				firstLine = false
				fmt.Fprintf(w, "%.02X ", vm.ram[i][j])
				fill += 3
				if j == 255 {
					i++
				}
				j++
				pos++
			}
		}

		for k := fill; k <= 32; k++ {
			fmt.Fprintf(w, " ")
		}

		fmt.Fprintf(w, "%.3d %.04d  %s\n", lid0, line, srcLine[line-1])
		if overline {
			fmt.Fprintf(w, "\n")
		}
		w.Flush()

		//fmt.Printf("p%d o%d l%d bl%d\n", i, j, line, blameLine[i][j])
		if lid[i] == 0 {
			i++
			j = 0
		}
		for blameLine[i][j] == 0 {
			if j == 255 {
				if i == 255 {
					return
				}
				i++
			}
			j++
			//fmt.Printf("Advancing: p%d o%d l%d bl%d\n", i, j, line, blameLine[i][j])
		}

	}
}

func wrVM() {
	f, e := os.Create("corestate.myst")
	if e != nil {
		log.Fatal("Could not create output file")
	}
	defer f.Close()
	e = binary.Write(f, binary.NativeEndian, vm)
	if e != nil {
		log.Fatal("Could not write output file")
	}
}

// Generate one byte of object code
// Increment lid (number of bytes of code assembled into a given page)
// Go to next page once current page is full

func putCode(b byte) {
	blameLine[page][offs] = lineNum
	vm.ram[page][offs] = b
	if offs == 255 {
		page++
	} else {
		lid[page]++
	}
	offs++
}

// Helper function to POC (Page label, Offset label, Constant label)
// Argument 'word' was checked to start with 'P[', 'O[', or 'C['
// Format is either x[label]value, or no value

func extractLabel(word string) (hasVal bool, label string, value byte) {

	// skip "P[" etc. by beginning at 2
	var i int
	for i = 2; i < len(word); i++ {
		if word[i] == ']' {
			break
		} else {
			label += string(word[i])
		}
	}
	i++
	hasVal = false
	if len(word[i:]) != 0 {
		if word[len(word)-1] == '+' {
			return hasVal, label, 0
		}
		hasVal = true
		for j := 0; j < len(symTab); j++ {
			if symTab[j].str == word[i:] {
				value = symTab[j].val
			}
		}
	}
	return hasVal, label, value
}

// Helper function which resolves the label 'str'
// to the offset it refers to, searching for its
// definition towards lower offsets
// (i.e. defined earlier in the source code)

func backRef(str string) bool {
	for j := offs; j >= 0; j-- {
		if str == offsLabel[page][j] {
			putCode(j)
			return true
		}
	}
	return false
}

// Helper function which resolves the label 'str'
// to the offset it refers to, searching for its
// definition towards higher offsets
// (i.e. defined later in the source code)

func fwdRef(str string) bool {
	var j int
	for j = int(offs); j != 256; j++ {
		if str == offsLabel[page][j] {
			putCode(byte(j))
			return true
		}
	}
	return false
}

// Helper function that generates object code
// in the form of ASCII character from a
// string literal

func putStr(str string) {
	for i := 0; i < len(str); i++ {
		putCode(str[i])
	}
}

// Checks the search string in 'word'
// against the array of predefined string literals

func tryPredefined(word string) bool {
	for j := 0; j < len(symTab); j++ { // Predefined
		var searchStr string
		if len(word) > 1 && word[len(word)-1] == ',' {
			searchStr = word[0 : len(word)-1]
		} else {
			searchStr = word
		}
		if symTab[j].str == searchStr {
			putCode(symTab[j].val)
			return true
		}
	}
	return false
}

// Checks the search string in 'word'
// against the array of source code defined constants

func tryConstDefined(word string) bool {
	for j := 0; j < constTopIndex; j++ { // Defined consts
		var searchStr string
		if len(word) > 1 && word[len(word)-1] == ',' {
			searchStr = word[0 : len(word)-1]
		} else {
			searchStr = word
		}
		if defConst[j].str == searchStr {
			putCode(defConst[j].val)
			return true
		}
	}
	return false
}

// Matches 'word' against the array of page labels

func tryPageLabelRef(word string) bool {
	for j := 0; j < 256; j++ { // Page labels refs
		var searchStr string
		if len(word) > 1 && word[len(word)-1] == ',' {
			searchStr = word[0 : len(word)-1]
		} else {
			searchStr = word
		}
		if searchStr != "" && pageLabel[j] == searchStr {
			putCode(byte(j))
			return true
		}
	}
	return false
}

// Matches 'word' against the array of offset labels

func tryOffsLabelRef(word string) bool {
	// References: three cases: < > .

	if len(word) > 1 {

		if word[0] == '<' {
			if backRef(word[1:]) {
				return true
			}

		} else if word[0] == '>' {
			if fwdRef(word[1:]) {
				return true
			}

		} else if strings.Contains(word, ".") { // page.offset format
			part := strings.Split(word, ".")
			for j := 0; j <= 255; j++ { // locate page label
				if part[0] == pageLabel[j] {
					for k := 0; k <= 255; k++ { // locate offset label
						if part[1] == offsLabel[j][k] {
							putCode(byte(k))
							return true
						}
					}
				}
			}
		}
	}
	return false
}

// See of 'word' is related to string literals ("a b c")

func tryStringRelated(word string) bool {

	if len(word) > 0 && word[0] == '"' && !insideString {
		if insideComment {
			return false
		}
		insideString = true
		//fmt.Printf("Strings on: %s final char:%c\n", word, word[len(word)-1])
		//Assemble part after ""
		if len(word) > 1 {
			if word[len(word)-1] == '"' {
				putStr(word[1 : len(word)-1])
			} else {
				putStr(word[1:])
			}
		} else {
			panic("Single double quote")
		}
		if word[len(word)-1] == '"' {
			//fmt.Printf("Strings off: %s\n", word)
			insideString = false // "0" etc.
		}
		return true
	}

	if len(word) > 0 && word[len(word)-1] == '"' && insideString {
		if insideComment {
			return false
		}
		insideString = false
		//fmt.Printf("Strings off: %s\n", word)
		//Assemble part before ""
		if len(word) > 1 {
			putStr(" " + word[:len(word)-1])
		}
		return true
	}

	if insideString {
		//Assemble whole word
		putStr(" " + word)
		return true
	}

	return false
}

// See if 'word' is related to comments (a b c)

func tryCommentRelated(word string) bool {
	if strings.Contains(word, "(") {
		if insideString || word[0] == '"' {
			return false
		}
		if word[len(word)-1] != ')' {
			insideComment = true
		}
		return true
	}

	if strings.Contains(word, ")") {
		if insideString || word[len(word)-1] == '"' {
			return false
		}
		insideComment = false
		return true
	}

	if insideComment == true {
		return true
	}

	return false
}

// Check if 'word' is a page label definition (P[label]val),
// offset label definition (O[label]val), or a constant
// label definition (C[label]val).

func tryPOC(word string) bool {

	var label string
	var value byte

	if strings.Contains(word, "P[") {
		hasVal, label, value = extractLabel(word)
		if hasVal {
			pageLabel[value] = label
			page = value
			offs = 0
		} else {
			if word[len(word)-1] == '+' { //Advance to next page
				page++
				offs = 0
			}
			pageLabel[page] = label
		}
		return true
	}

	if strings.Contains(word, "O[") {
		hasVal, label, value = extractLabel(word)
		if hasVal {
			offsLabel[page][value] = label
			offs = value
			if offs > lid[page] {
				lid[page] = offs
			}
		} else {
			offsLabel[page][offs] = label
		}
		return true
	}

	if strings.Contains(word, "C[") {
		hasVal, label, value = extractLabel(word)
		if hasVal == false {
			panic("Const has no value")
		}
		defConst[constTopIndex].str = label
		defConst[constTopIndex].val = value
		constTopIndex++
		return true
	}

	return false
}

// Check if 'word' refers to a trap call / has a leading asterisk

func tryTrapCall(word string) bool {
	if len(word) > 0 && word[0] == '*' { //Trap call
		for i := 0; i < 32; i++ {
			if pageLabel[i] == word[1:] {
				putCode(byte(i) & 0x40)
				return true
			}
		}
	}
	return false
}

var hasVal bool // Silence "unused" warning/err if I put this inside the function!?
func parse(pass byte) {

	var wordIndex int

	fmt.Printf("Pass %d\n", pass)

	for lineNum = 1; lineNum < len(srcLine); lineNum++ {
		wordsInLine := strings.Fields(srcLine[lineNum-1])

		wordIndex = 0
		for wordIndex < len(wordsInLine) {
			word := wordsInLine[wordIndex]
			if len(word) == 0 {
				continue // Empty source line!
			} else {
				if word[0] == ';' { //Ignore entire line
					break
				}
			}

			if word[len(word)-1] == ',' {
				word = word[:len(word)-1] // Chop
			}

			if tryCommentRelated(word) {
				wordIndex++
				continue
			}

			if tryOffsLabelRef(word) {
				wordIndex++
				continue
			}

			if tryPOC(word) {
				wordIndex++
				continue
			}

			if tryPredefined(word) {
				wordIndex++
				continue
			}

			if tryConstDefined(word) {
				wordIndex++
				continue
			}

			if tryPageLabelRef(word) {
				wordIndex++
				continue
			}

			if tryTrapCall(word) {
				wordIndex++
				continue
			}

			if tryStringRelated(word) {
				wordIndex++
				continue
			}

			wordIndex++
			if pass == 2 {
				fmt.Printf("Line %d: Using 0 for unknown literal '%s'\n", lineNum, word)
			}
			putCode(0)
			continue
		}
	}
}

func main() {

	if len(os.Args[1:]) < 1 {
		fmt.Println("Missing input file arguments <asmsrc>")
		os.Exit(1)
	}

	fmt.Println("Goldie/LOX assembler v.2409")

	vm = myth_vm{}

	srcText := ldSrc(os.Args[1:][0])
	srcTextStr := string(srcText)
	srcLine = strings.Split(srcTextStr, "\n")
	fmt.Printf("Source file has %d lines\n", len(srcLine))

	parse(1)
	for i := 0; i < 256; i++ {
		lid[i] = 0
	}
	parse(2) // Second Pass to resolve forward refs

	wrVM()
	wrDebugTxt()
	fmt.Println("Goldie/LOX finished")
}
