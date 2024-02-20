
/* Sonne Microcontroller rev. Myth
   Virtual Machine
  Jan-2024 Michael Mangelsdorf
  Copyr. <mim@ok-schalter.de>
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LHS_N     0
#define LHS_M     1
#define LHS_D     2
#define LHS_O     3
#define LHS_R     4
#define LHS_I     5
#define LHS_S     6
#define LHS_P     7

#define RHS_G     0
#define RHS_M     1
#define RHS_D     2
#define RHS_O     3
#define RHS_R     4
#define RHS_I     5
#define RHS_S     6
#define RHS_P     7
#define RHS_E     8
#define RHS_X     9
#define RHS_J     10
#define RHS_T     11
#define RHS_F     12
#define RHS_C     13
#define RHS_A     14
#define RHS_B     15

#define ALU_IDA 0 /*Identity A*/
#define ALU_IDB 1 /*Identity B*/
#define ALU_OCA 2 /*One's complement A*/
#define ALU_OCB 3 /*One's complement B*/
#define ALU_SLA 4 /*Shift left A*/
#define ALU_SLB 5 /*Shift left B*/
#define ALU_SRA 6 /*Shift right A*/
#define ALU_SRB 7 /*Shift right B*/
#define ALU_AND 8 /*A AND B*/
#define ALU_IOR 9 /*A OR B*/
#define ALU_EOR 10 /*A XOR B*/
#define ALU_ADD 11 /*A + B*/
#define ALU_CAR 12 /*Carry of A + B (0 or 1)*/
#define ALU_ALB 13 /*255 if A<B else 0*/
#define ALU_AEB 14 /*255 if A=B else 0*/
#define ALU_AGB 15 /*255 if A>B else 0*/

#define SYS_NOP 0 /*No operation*/
#define SYS_SSI 1 /*Serial Shift In*/
#define SYS_SSO 2 /*Serial Shift Out*/
#define SYS_SCL 3 /*Set serial Clock Low*/
#define SYS_SCH 4 /*Set serial Clock High*/
#define SYS_RDY 5 /*Ready/Tristate*/
#define SYS_NEW 6 /*Create stack frame*/
#define SYS_OLD 7 /*Resume stack frame*/


const char *myth_mnemonics[256] = {
"NOP", "SSI", "SSO", "SCL", "SCH", "RDY", "NEW", "OLD", 
"P4", "P1", "P2", "P3", "M4", "M3", "M2", "M1", 
"IDA", "IDB", "OCA", "OCB", "SLA", "SLB", "SRA", "SRB", 
"AND", "IOR", "EOR", "ADD", "CAR", "ALB", "AEB", "AGB", 
"*0", "*1", "*2", "*3", "*4", "*5", "*6", "*7", 
"*8", "*9", "*10", "*11", "*12", "*13", "*14", "*15", 
"*16", "*17", "*18", "*19", "*20", "*21", "*22", "*23", 
"*24", "*25", "*26", "*27", "*28", "*29", "*30", "*31", 
"G0a", "G0b", "G0r", "G0i", "L0a", "L0b", "L0r", "L0i", 
"aG0", "bG0", "rG0", "iG0", "aL0", "bL0", "rL0", "iL0", 
"G1a", "G1b", "G1r", "G1i", "L1a", "L1b", "L1r", "L1i", 
"aG1", "bG1", "rG1", "iG1", "aL1", "bL1", "rL1", "iL1", 
"G2a", "G2b", "G2r", "G2i", "L2a", "L2b", "L2r", "L2i", 
"aG2", "bG2", "rG2", "iG2", "aL2", "bL2", "rL2", "iL2", 
"G3a", "G3b", "G3r", "G3i", "L3a", "L3b", "L3r", "L3i", 
"aG3", "bG3", "rG3", "iG3", "aL3", "bL3", "rL3", "iL3", 
"NG", "RET", "ND", "NO", "NR", "NI", "NS", "NP", 
"NE", "NX", "NJ", "NT", "NF", "NC", "NA", "NB", 
"MG", "CLR", "MD", "MO", "MR", "MI", "MS", "MP", 
"ME", "MX", "MJ", "MT", "MF", "MC", "MA", "MB", 
"DG", "DM", "DD", "DO", "DR", "DI", "DS", "DP", 
"DE", "DX", "DJ", "DT", "DF", "BIO", "DA", "DB", 
"OG", "OM", "OD", "OO", "OR", "OI", "OS", "OP", 
"OE", "OX", "OJ", "OT", "OF", "OC", "OA", "OB", 
"RG", "RM", "RD", "RO", "RR", "RI", "RS", "RP", 
"RE", "RX", "RJ", "RT", "RF", "CPI", "RA", "RB", 
"IG", "IM", "ID", "IO", "IR", "II", "IS", "IP", 
"IE", "IX", "IJ", "IT", "IF", "IC", "IA", "IB", 
"SG", "SM", "SD", "SO", "SR", "SI", "SS", "SP", 
"SE", "SX", "SJ", "ST", "SF", "SC", "SA", "SB", 
"PG", "PM", "PD", "PO", "PR", "PI", "PS", "PP", 
"PE", "PX", "PJ", "PT", "PF", "PC", "PA", "PB"
};

#define NUMBERS_ARRAY_SIZE 928
struct {char *name; uint8_t val;} myth_predefs[NUMBERS_ARRAY_SIZE] = {
{"0",0x00}, {"1",0x01}, {"2",0x02}, {"3",0x03}, 
{"4",0x04}, {"5",0x05}, {"6",0x06}, {"7",0x07}, 
{"8",0x08}, {"9",0x09}, {"10",0x0A}, {"11",0x0B}, 
{"12",0x0C}, {"13",0x0D}, {"14",0x0E}, {"15",0x0F}, 
{"16",0x10}, {"17",0x11}, {"18",0x12}, {"19",0x13}, 
{"20",0x14}, {"21",0x15}, {"22",0x16}, {"23",0x17}, 
{"24",0x18}, {"25",0x19}, {"26",0x1A}, {"27",0x1B}, 
{"28",0x1C}, {"29",0x1D}, {"30",0x1E}, {"31",0x1F}, 
{"32",0x20}, {"33",0x21}, {"34",0x22}, {"35",0x23}, 
{"36",0x24}, {"37",0x25}, {"38",0x26}, {"39",0x27}, 
{"40",0x28}, {"41",0x29}, {"42",0x2A}, {"43",0x2B}, 
{"44",0x2C}, {"45",0x2D}, {"46",0x2E}, {"47",0x2F}, 
{"48",0x30}, {"49",0x31}, {"50",0x32}, {"51",0x33}, 
{"52",0x34}, {"53",0x35}, {"54",0x36}, {"55",0x37}, 
{"56",0x38}, {"57",0x39}, {"58",0x3A}, {"59",0x3B}, 
{"60",0x3C}, {"61",0x3D}, {"62",0x3E}, {"63",0x3F}, 
{"64",0x40}, {"65",0x41}, {"66",0x42}, {"67",0x43}, 
{"68",0x44}, {"69",0x45}, {"70",0x46}, {"71",0x47}, 
{"72",0x48}, {"73",0x49}, {"74",0x4A}, {"75",0x4B}, 
{"76",0x4C}, {"77",0x4D}, {"78",0x4E}, {"79",0x4F}, 
{"80",0x50}, {"81",0x51}, {"82",0x52}, {"83",0x53}, 
{"84",0x54}, {"85",0x55}, {"86",0x56}, {"87",0x57}, 
{"88",0x58}, {"89",0x59}, {"90",0x5A}, {"91",0x5B}, 
{"92",0x5C}, {"93",0x5D}, {"94",0x5E}, {"95",0x5F}, 
{"96",0x60}, {"97",0x61}, {"98",0x62}, {"99",0x63}, 
{"100",0x64}, {"101",0x65}, {"102",0x66}, {"103",0x67}, 
{"104",0x68}, {"105",0x69}, {"106",0x6A}, {"107",0x6B}, 
{"108",0x6C}, {"109",0x6D}, {"110",0x6E}, {"111",0x6F}, 
{"112",0x70}, {"113",0x71}, {"114",0x72}, {"115",0x73}, 
{"116",0x74}, {"117",0x75}, {"118",0x76}, {"119",0x77}, 
{"120",0x78}, {"121",0x79}, {"122",0x7A}, {"123",0x7B}, 
{"124",0x7C}, {"125",0x7D}, {"126",0x7E}, {"127",0x7F}, 
{"128",0x80}, {"129",0x81}, {"130",0x82}, {"131",0x83}, 
{"132",0x84}, {"133",0x85}, {"134",0x86}, {"135",0x87}, 
{"136",0x88}, {"137",0x89}, {"138",0x8A}, {"139",0x8B}, 
{"140",0x8C}, {"141",0x8D}, {"142",0x8E}, {"143",0x8F}, 
{"144",0x90}, {"145",0x91}, {"146",0x92}, {"147",0x93}, 
{"148",0x94}, {"149",0x95}, {"150",0x96}, {"151",0x97}, 
{"152",0x98}, {"153",0x99}, {"154",0x9A}, {"155",0x9B}, 
{"156",0x9C}, {"157",0x9D}, {"158",0x9E}, {"159",0x9F}, 
{"160",0xA0}, {"161",0xA1}, {"162",0xA2}, {"163",0xA3}, 
{"164",0xA4}, {"165",0xA5}, {"166",0xA6}, {"167",0xA7}, 
{"168",0xA8}, {"169",0xA9}, {"170",0xAA}, {"171",0xAB}, 
{"172",0xAC}, {"173",0xAD}, {"174",0xAE}, {"175",0xAF}, 
{"176",0xB0}, {"177",0xB1}, {"178",0xB2}, {"179",0xB3}, 
{"180",0xB4}, {"181",0xB5}, {"182",0xB6}, {"183",0xB7}, 
{"184",0xB8}, {"185",0xB9}, {"186",0xBA}, {"187",0xBB}, 
{"188",0xBC}, {"189",0xBD}, {"190",0xBE}, {"191",0xBF}, 
{"192",0xC0}, {"193",0xC1}, {"194",0xC2}, {"195",0xC3}, 
{"196",0xC4}, {"197",0xC5}, {"198",0xC6}, {"199",0xC7}, 
{"200",0xC8}, {"201",0xC9}, {"202",0xCA}, {"203",0xCB}, 
{"204",0xCC}, {"205",0xCD}, {"206",0xCE}, {"207",0xCF}, 
{"208",0xD0}, {"209",0xD1}, {"210",0xD2}, {"211",0xD3}, 
{"212",0xD4}, {"213",0xD5}, {"214",0xD6}, {"215",0xD7}, 
{"216",0xD8}, {"217",0xD9}, {"218",0xDA}, {"219",0xDB}, 
{"220",0xDC}, {"221",0xDD}, {"222",0xDE}, {"223",0xDF}, 
{"224",0xE0}, {"225",0xE1}, {"226",0xE2}, {"227",0xE3}, 
{"228",0xE4}, {"229",0xE5}, {"230",0xE6}, {"231",0xE7}, 
{"232",0xE8}, {"233",0xE9}, {"234",0xEA}, {"235",0xEB}, 
{"236",0xEC}, {"237",0xED}, {"238",0xEE}, {"239",0xEF}, 
{"240",0xF0}, {"241",0xF1}, {"242",0xF2}, {"243",0xF3}, 
{"244",0xF4}, {"245",0xF5}, {"246",0xF6}, {"247",0xF7}, 
{"248",0xF8}, {"249",0xF9}, {"250",0xFA}, {"251",0xFB}, 
{"252",0xFC}, {"253",0xFD}, {"254",0xFE}, {"255",0xFF}, 
{"-1",0xFF}, {"-2",0xFE}, {"-3",0xFD}, {"-4",0xFC}, 
{"-5",0xFB}, {"-6",0xFA}, {"-7",0xF9}, {"-8",0xF8}, 
{"-9",0xF7}, {"-10",0xF6}, {"-11",0xF5}, {"-12",0xF4}, 
{"-13",0xF3}, {"-14",0xF2}, {"-15",0xF1}, {"-16",0xF0}, 
{"-17",0xEF}, {"-18",0xEE}, {"-19",0xED}, {"-20",0xEC}, 
{"-21",0xEB}, {"-22",0xEA}, {"-23",0xE9}, {"-24",0xE8}, 
{"-25",0xE7}, {"-26",0xE6}, {"-27",0xE5}, {"-28",0xE4}, 
{"-29",0xE3}, {"-30",0xE2}, {"-31",0xE1}, {"-32",0xE0}, 
{"-33",0xDF}, {"-34",0xDE}, {"-35",0xDD}, {"-36",0xDC}, 
{"-37",0xDB}, {"-38",0xDA}, {"-39",0xD9}, {"-40",0xD8}, 
{"-41",0xD7}, {"-42",0xD6}, {"-43",0xD5}, {"-44",0xD4}, 
{"-45",0xD3}, {"-46",0xD2}, {"-47",0xD1}, {"-48",0xD0}, 
{"-49",0xCF}, {"-50",0xCE}, {"-51",0xCD}, {"-52",0xCC}, 
{"-53",0xCB}, {"-54",0xCA}, {"-55",0xC9}, {"-56",0xC8}, 
{"-57",0xC7}, {"-58",0xC6}, {"-59",0xC5}, {"-60",0xC4}, 
{"-61",0xC3}, {"-62",0xC2}, {"-63",0xC1}, {"-64",0xC0}, 
{"-65",0xBF}, {"-66",0xBE}, {"-67",0xBD}, {"-68",0xBC}, 
{"-69",0xBB}, {"-70",0xBA}, {"-71",0xB9}, {"-72",0xB8}, 
{"-73",0xB7}, {"-74",0xB6}, {"-75",0xB5}, {"-76",0xB4}, 
{"-77",0xB3}, {"-78",0xB2}, {"-79",0xB1}, {"-80",0xB0}, 
{"-81",0xAF}, {"-82",0xAE}, {"-83",0xAD}, {"-84",0xAC}, 
{"-85",0xAB}, {"-86",0xAA}, {"-87",0xA9}, {"-88",0xA8}, 
{"-89",0xA7}, {"-90",0xA6}, {"-91",0xA5}, {"-92",0xA4}, 
{"-93",0xA3}, {"-94",0xA2}, {"-95",0xA1}, {"-96",0xA0}, 
{"-97",0x9F}, {"-98",0x9E}, {"-99",0x9D}, {"-100",0x9C}, 
{"-101",0x9B}, {"-102",0x9A}, {"-103",0x99}, {"-104",0x98}, 
{"-105",0x97}, {"-106",0x96}, {"-107",0x95}, {"-108",0x94}, 
{"-109",0x93}, {"-110",0x92}, {"-111",0x91}, {"-112",0x90}, 
{"-113",0x8F}, {"-114",0x8E}, {"-115",0x8D}, {"-116",0x8C}, 
{"-117",0x8B}, {"-118",0x8A}, {"-119",0x89}, {"-120",0x88}, 
{"-121",0x87}, {"-122",0x86}, {"-123",0x85}, {"-124",0x84}, 
{"-125",0x83}, {"-126",0x82}, {"-127",0x81}, {"-128",0x80}, 
{"00h",0x00}, {"01h",0x01}, {"02h",0x02}, {"03h",0x03}, 
{"04h",0x04}, {"05h",0x05}, {"06h",0x06}, {"07h",0x07}, 
{"08h",0x08}, {"09h",0x09}, {"0Ah",0x0A}, {"0Bh",0x0B}, 
{"0Ch",0x0C}, {"0Dh",0x0D}, {"0Eh",0x0E}, {"0Fh",0x0F}, 
{"10h",0x10}, {"11h",0x11}, {"12h",0x12}, {"13h",0x13}, 
{"14h",0x14}, {"15h",0x15}, {"16h",0x16}, {"17h",0x17}, 
{"18h",0x18}, {"19h",0x19}, {"1Ah",0x1A}, {"1Bh",0x1B}, 
{"1Ch",0x1C}, {"1Dh",0x1D}, {"1Eh",0x1E}, {"1Fh",0x1F}, 
{"20h",0x20}, {"21h",0x21}, {"22h",0x22}, {"23h",0x23}, 
{"24h",0x24}, {"25h",0x25}, {"26h",0x26}, {"27h",0x27}, 
{"28h",0x28}, {"29h",0x29}, {"2Ah",0x2A}, {"2Bh",0x2B}, 
{"2Ch",0x2C}, {"2Dh",0x2D}, {"2Eh",0x2E}, {"2Fh",0x2F}, 
{"30h",0x30}, {"31h",0x31}, {"32h",0x32}, {"33h",0x33}, 
{"34h",0x34}, {"35h",0x35}, {"36h",0x36}, {"37h",0x37}, 
{"38h",0x38}, {"39h",0x39}, {"3Ah",0x3A}, {"3Bh",0x3B}, 
{"3Ch",0x3C}, {"3Dh",0x3D}, {"3Eh",0x3E}, {"3Fh",0x3F}, 
{"40h",0x40}, {"41h",0x41}, {"42h",0x42}, {"43h",0x43}, 
{"44h",0x44}, {"45h",0x45}, {"46h",0x46}, {"47h",0x47}, 
{"48h",0x48}, {"49h",0x49}, {"4Ah",0x4A}, {"4Bh",0x4B}, 
{"4Ch",0x4C}, {"4Dh",0x4D}, {"4Eh",0x4E}, {"4Fh",0x4F}, 
{"50h",0x50}, {"51h",0x51}, {"52h",0x52}, {"53h",0x53}, 
{"54h",0x54}, {"55h",0x55}, {"56h",0x56}, {"57h",0x57}, 
{"58h",0x58}, {"59h",0x59}, {"5Ah",0x5A}, {"5Bh",0x5B}, 
{"5Ch",0x5C}, {"5Dh",0x5D}, {"5Eh",0x5E}, {"5Fh",0x5F}, 
{"60h",0x60}, {"61h",0x61}, {"62h",0x62}, {"63h",0x63}, 
{"64h",0x64}, {"65h",0x65}, {"66h",0x66}, {"67h",0x67}, 
{"68h",0x68}, {"69h",0x69}, {"6Ah",0x6A}, {"6Bh",0x6B}, 
{"6Ch",0x6C}, {"6Dh",0x6D}, {"6Eh",0x6E}, {"6Fh",0x6F}, 
{"70h",0x70}, {"71h",0x71}, {"72h",0x72}, {"73h",0x73}, 
{"74h",0x74}, {"75h",0x75}, {"76h",0x76}, {"77h",0x77}, 
{"78h",0x78}, {"79h",0x79}, {"7Ah",0x7A}, {"7Bh",0x7B}, 
{"7Ch",0x7C}, {"7Dh",0x7D}, {"7Eh",0x7E}, {"7Fh",0x7F}, 
{"80h",0x80}, {"81h",0x81}, {"82h",0x82}, {"83h",0x83}, 
{"84h",0x84}, {"85h",0x85}, {"86h",0x86}, {"87h",0x87}, 
{"88h",0x88}, {"89h",0x89}, {"8Ah",0x8A}, {"8Bh",0x8B}, 
{"8Ch",0x8C}, {"8Dh",0x8D}, {"8Eh",0x8E}, {"8Fh",0x8F}, 
{"90h",0x90}, {"91h",0x91}, {"92h",0x92}, {"93h",0x93}, 
{"94h",0x94}, {"95h",0x95}, {"96h",0x96}, {"97h",0x97}, 
{"98h",0x98}, {"99h",0x99}, {"9Ah",0x9A}, {"9Bh",0x9B}, 
{"9Ch",0x9C}, {"9Dh",0x9D}, {"9Eh",0x9E}, {"9Fh",0x9F}, 
{"A0h",0xA0}, {"A1h",0xA1}, {"A2h",0xA2}, {"A3h",0xA3}, 
{"A4h",0xA4}, {"A5h",0xA5}, {"A6h",0xA6}, {"A7h",0xA7}, 
{"A8h",0xA8}, {"A9h",0xA9}, {"AAh",0xAA}, {"ABh",0xAB}, 
{"ACh",0xAC}, {"ADh",0xAD}, {"AEh",0xAE}, {"AFh",0xAF}, 
{"B0h",0xB0}, {"B1h",0xB1}, {"B2h",0xB2}, {"B3h",0xB3}, 
{"B4h",0xB4}, {"B5h",0xB5}, {"B6h",0xB6}, {"B7h",0xB7}, 
{"B8h",0xB8}, {"B9h",0xB9}, {"BAh",0xBA}, {"BBh",0xBB}, 
{"BCh",0xBC}, {"BDh",0xBD}, {"BEh",0xBE}, {"BFh",0xBF}, 
{"C0h",0xC0}, {"C1h",0xC1}, {"C2h",0xC2}, {"C3h",0xC3}, 
{"C4h",0xC4}, {"C5h",0xC5}, {"C6h",0xC6}, {"C7h",0xC7}, 
{"C8h",0xC8}, {"C9h",0xC9}, {"CAh",0xCA}, {"CBh",0xCB}, 
{"CCh",0xCC}, {"CDh",0xCD}, {"CEh",0xCE}, {"CFh",0xCF}, 
{"D0h",0xD0}, {"D1h",0xD1}, {"D2h",0xD2}, {"D3h",0xD3}, 
{"D4h",0xD4}, {"D5h",0xD5}, {"D6h",0xD6}, {"D7h",0xD7}, 
{"D8h",0xD8}, {"D9h",0xD9}, {"DAh",0xDA}, {"DBh",0xDB}, 
{"DCh",0xDC}, {"DDh",0xDD}, {"DEh",0xDE}, {"DFh",0xDF}, 
{"E0h",0xE0}, {"E1h",0xE1}, {"E2h",0xE2}, {"E3h",0xE3}, 
{"E4h",0xE4}, {"E5h",0xE5}, {"E6h",0xE6}, {"E7h",0xE7}, 
{"E8h",0xE8}, {"E9h",0xE9}, {"EAh",0xEA}, {"EBh",0xEB}, 
{"ECh",0xEC}, {"EDh",0xED}, {"EEh",0xEE}, {"EFh",0xEF}, 
{"F0h",0xF0}, {"F1h",0xF1}, {"F2h",0xF2}, {"F3h",0xF3}, 
{"F4h",0xF4}, {"F5h",0xF5}, {"F6h",0xF6}, {"F7h",0xF7}, 
{"F8h",0xF8}, {"F9h",0xF9}, {"FAh",0xFA}, {"FBh",0xFB}, 
{"FCh",0xFC}, {"FDh",0xFD}, {"FEh",0xFE}, {"FFh",0xFF}, 
{"0h",0x00}, {"1h",0x01}, {"2h",0x02}, {"3h",0x03}, 
{"4h",0x04}, {"5h",0x05}, {"6h",0x06}, {"7h",0x07}, 
{"8h",0x08}, {"9h",0x09}, {"Ah",0x0A}, {"Bh",0x0B}, 
{"Ch",0x0C}, {"Dh",0x0D}, {"Eh",0x0E}, {"Fh",0x0F}, 
{"0000_0000b",0x00}, {"0000_0001b",0x01}, {"0000_0010b",0x02}, {"0000_0011b",0x03}, 
{"0000_0100b",0x04}, {"0000_0101b",0x05}, {"0000_0110b",0x06}, {"0000_0111b",0x07}, 
{"0000_1000b",0x08}, {"0000_1001b",0x09}, {"0000_1010b",0x0A}, {"0000_1011b",0x0B}, 
{"0000_1100b",0x0C}, {"0000_1101b",0x0D}, {"0000_1110b",0x0E}, {"0000_1111b",0x0F}, 
{"0001_0000b",0x10}, {"0001_0001b",0x11}, {"0001_0010b",0x12}, {"0001_0011b",0x13}, 
{"0001_0100b",0x14}, {"0001_0101b",0x15}, {"0001_0110b",0x16}, {"0001_0111b",0x17}, 
{"0001_1000b",0x18}, {"0001_1001b",0x19}, {"0001_1010b",0x1A}, {"0001_1011b",0x1B}, 
{"0001_1100b",0x1C}, {"0001_1101b",0x1D}, {"0001_1110b",0x1E}, {"0001_1111b",0x1F}, 
{"0010_0000b",0x20}, {"0010_0001b",0x21}, {"0010_0010b",0x22}, {"0010_0011b",0x23}, 
{"0010_0100b",0x24}, {"0010_0101b",0x25}, {"0010_0110b",0x26}, {"0010_0111b",0x27}, 
{"0010_1000b",0x28}, {"0010_1001b",0x29}, {"0010_1010b",0x2A}, {"0010_1011b",0x2B}, 
{"0010_1100b",0x2C}, {"0010_1101b",0x2D}, {"0010_1110b",0x2E}, {"0010_1111b",0x2F}, 
{"0011_0000b",0x30}, {"0011_0001b",0x31}, {"0011_0010b",0x32}, {"0011_0011b",0x33}, 
{"0011_0100b",0x34}, {"0011_0101b",0x35}, {"0011_0110b",0x36}, {"0011_0111b",0x37}, 
{"0011_1000b",0x38}, {"0011_1001b",0x39}, {"0011_1010b",0x3A}, {"0011_1011b",0x3B}, 
{"0011_1100b",0x3C}, {"0011_1101b",0x3D}, {"0011_1110b",0x3E}, {"0011_1111b",0x3F}, 
{"0100_0000b",0x40}, {"0100_0001b",0x41}, {"0100_0010b",0x42}, {"0100_0011b",0x43}, 
{"0100_0100b",0x44}, {"0100_0101b",0x45}, {"0100_0110b",0x46}, {"0100_0111b",0x47}, 
{"0100_1000b",0x48}, {"0100_1001b",0x49}, {"0100_1010b",0x4A}, {"0100_1011b",0x4B}, 
{"0100_1100b",0x4C}, {"0100_1101b",0x4D}, {"0100_1110b",0x4E}, {"0100_1111b",0x4F}, 
{"0101_0000b",0x50}, {"0101_0001b",0x51}, {"0101_0010b",0x52}, {"0101_0011b",0x53}, 
{"0101_0100b",0x54}, {"0101_0101b",0x55}, {"0101_0110b",0x56}, {"0101_0111b",0x57}, 
{"0101_1000b",0x58}, {"0101_1001b",0x59}, {"0101_1010b",0x5A}, {"0101_1011b",0x5B}, 
{"0101_1100b",0x5C}, {"0101_1101b",0x5D}, {"0101_1110b",0x5E}, {"0101_1111b",0x5F}, 
{"0110_0000b",0x60}, {"0110_0001b",0x61}, {"0110_0010b",0x62}, {"0110_0011b",0x63}, 
{"0110_0100b",0x64}, {"0110_0101b",0x65}, {"0110_0110b",0x66}, {"0110_0111b",0x67}, 
{"0110_1000b",0x68}, {"0110_1001b",0x69}, {"0110_1010b",0x6A}, {"0110_1011b",0x6B}, 
{"0110_1100b",0x6C}, {"0110_1101b",0x6D}, {"0110_1110b",0x6E}, {"0110_1111b",0x6F}, 
{"0111_0000b",0x70}, {"0111_0001b",0x71}, {"0111_0010b",0x72}, {"0111_0011b",0x73}, 
{"0111_0100b",0x74}, {"0111_0101b",0x75}, {"0111_0110b",0x76}, {"0111_0111b",0x77}, 
{"0111_1000b",0x78}, {"0111_1001b",0x79}, {"0111_1010b",0x7A}, {"0111_1011b",0x7B}, 
{"0111_1100b",0x7C}, {"0111_1101b",0x7D}, {"0111_1110b",0x7E}, {"0111_1111b",0x7F}, 
{"1000_0000b",0x80}, {"1000_0001b",0x81}, {"1000_0010b",0x82}, {"1000_0011b",0x83}, 
{"1000_0100b",0x84}, {"1000_0101b",0x85}, {"1000_0110b",0x86}, {"1000_0111b",0x87}, 
{"1000_1000b",0x88}, {"1000_1001b",0x89}, {"1000_1010b",0x8A}, {"1000_1011b",0x8B}, 
{"1000_1100b",0x8C}, {"1000_1101b",0x8D}, {"1000_1110b",0x8E}, {"1000_1111b",0x8F}, 
{"1001_0000b",0x90}, {"1001_0001b",0x91}, {"1001_0010b",0x92}, {"1001_0011b",0x93}, 
{"1001_0100b",0x94}, {"1001_0101b",0x95}, {"1001_0110b",0x96}, {"1001_0111b",0x97}, 
{"1001_1000b",0x98}, {"1001_1001b",0x99}, {"1001_1010b",0x9A}, {"1001_1011b",0x9B}, 
{"1001_1100b",0x9C}, {"1001_1101b",0x9D}, {"1001_1110b",0x9E}, {"1001_1111b",0x9F}, 
{"1010_0000b",0xA0}, {"1010_0001b",0xA1}, {"1010_0010b",0xA2}, {"1010_0011b",0xA3}, 
{"1010_0100b",0xA4}, {"1010_0101b",0xA5}, {"1010_0110b",0xA6}, {"1010_0111b",0xA7}, 
{"1010_1000b",0xA8}, {"1010_1001b",0xA9}, {"1010_1010b",0xAA}, {"1010_1011b",0xAB}, 
{"1010_1100b",0xAC}, {"1010_1101b",0xAD}, {"1010_1110b",0xAE}, {"1010_1111b",0xAF}, 
{"1011_0000b",0xB0}, {"1011_0001b",0xB1}, {"1011_0010b",0xB2}, {"1011_0011b",0xB3}, 
{"1011_0100b",0xB4}, {"1011_0101b",0xB5}, {"1011_0110b",0xB6}, {"1011_0111b",0xB7}, 
{"1011_1000b",0xB8}, {"1011_1001b",0xB9}, {"1011_1010b",0xBA}, {"1011_1011b",0xBB}, 
{"1011_1100b",0xBC}, {"1011_1101b",0xBD}, {"1011_1110b",0xBE}, {"1011_1111b",0xBF}, 
{"1100_0000b",0xC0}, {"1100_0001b",0xC1}, {"1100_0010b",0xC2}, {"1100_0011b",0xC3}, 
{"1100_0100b",0xC4}, {"1100_0101b",0xC5}, {"1100_0110b",0xC6}, {"1100_0111b",0xC7}, 
{"1100_1000b",0xC8}, {"1100_1001b",0xC9}, {"1100_1010b",0xCA}, {"1100_1011b",0xCB}, 
{"1100_1100b",0xCC}, {"1100_1101b",0xCD}, {"1100_1110b",0xCE}, {"1100_1111b",0xCF}, 
{"1101_0000b",0xD0}, {"1101_0001b",0xD1}, {"1101_0010b",0xD2}, {"1101_0011b",0xD3}, 
{"1101_0100b",0xD4}, {"1101_0101b",0xD5}, {"1101_0110b",0xD6}, {"1101_0111b",0xD7}, 
{"1101_1000b",0xD8}, {"1101_1001b",0xD9}, {"1101_1010b",0xDA}, {"1101_1011b",0xDB}, 
{"1101_1100b",0xDC}, {"1101_1101b",0xDD}, {"1101_1110b",0xDE}, {"1101_1111b",0xDF}, 
{"1110_0000b",0xE0}, {"1110_0001b",0xE1}, {"1110_0010b",0xE2}, {"1110_0011b",0xE3}, 
{"1110_0100b",0xE4}, {"1110_0101b",0xE5}, {"1110_0110b",0xE6}, {"1110_0111b",0xE7}, 
{"1110_1000b",0xE8}, {"1110_1001b",0xE9}, {"1110_1010b",0xEA}, {"1110_1011b",0xEB}, 
{"1110_1100b",0xEC}, {"1110_1101b",0xED}, {"1110_1110b",0xEE}, {"1110_1111b",0xEF}, 
{"1111_0000b",0xF0}, {"1111_0001b",0xF1}, {"1111_0010b",0xF2}, {"1111_0011b",0xF3}, 
{"1111_0100b",0xF4}, {"1111_0101b",0xF5}, {"1111_0110b",0xF6}, {"1111_0111b",0xF7}, 
{"1111_1000b",0xF8}, {"1111_1001b",0xF9}, {"1111_1010b",0xFA}, {"1111_1011b",0xFB}, 
{"1111_1100b",0xFC}, {"1111_1101b",0xFD}, {"1111_1110b",0xFE}, {"1111_1111b",0xFF}, 
{"0000b",0x00}, {"0001b",0x01}, {"0010b",0x02}, {"0011b",0x03}, 
{"0100b",0x04}, {"0101b",0x05}, {"0110b",0x06}, {"0111b",0x07}, 
{"1000b",0x08}, {"1001b",0x09}, {"1010b",0x0A}, {"1011b",0x0B}, 
{"1100b",0x0C}, {"1101b",0x0D}, {"1110b",0x0E}, {"1111b",0x0F}
};

char tempStr80[81];

struct myth_vm {

    uint8_t reg_G;
    uint8_t reg_D;
    uint8_t reg_O;
    uint8_t reg_R;
    uint8_t reg_I;
    uint8_t reg_A;
    uint8_t reg_B;
    uint8_t reg_BIO;
    uint8_t reg_C;
    uint8_t reg_L;
    uint8_t reg_E;
    uint8_t reg_SIR;
    uint8_t reg_SOR;
    uint8_t reg_PIR;
    uint8_t reg_POR;
    uint8_t reg_PC;
    uint8_t reg_xMx;

    uint8_t par_ready;
    uint8_t ser_clock;

    uint8_t pagebuf[256];
    uint8_t ram[0x8000];
    
    struct {
        char name[13];
        uint8_t page_idx;
        uint8_t byte_idx;
        uint8_t value;
    } labels;

    void (*E_posedge[16])(struct myth_vm*);
    void (*E_negedge[16])(struct myth_vm*);
} vm;


typedef struct myth_vm Myth_vm;

uint8_t quitf, loggingf;


char* binStr(unsigned char byte) {
    int i, pos=0;
    for (i = 7; i >= 0; i--) {
        tempStr80[pos++] = ((byte >> i) & 1) ? '1' : '0';
        if (pos==4) {
            tempStr80[pos] = '_';
            pos++;
        }
    }
    tempStr80[9] = '\0';
    return tempStr80;
}


void
myth_set_A( Myth_vm *vm, uint8_t val)
{
    vm->reg_A = val;
    vm->reg_xMx = vm->reg_A;
}

void
myth_set_B( Myth_vm *vm, uint8_t val)
{
    vm->reg_B = val;
    vm->reg_xMx = vm->reg_B;
}


void
shift_in( Myth_vm *vm)
{
    uint8_t bit = 1; // Physical bit in
    vm->reg_SIR <<= 1; // MSB in first
    vm->reg_SIR |= bit;
}

void
shift_out( Myth_vm *vm)
{
    uint8_t bit = (vm->reg_SOR & 128) >> 7; // MSB out first
    vm->reg_SOR <<= 1;
}


void
myth_ALU( Myth_vm *vm, uint8_t func)
{
    switch(func & 15)
    {
        case ALU_IDA: vm->reg_R = vm->reg_A; break;
        case ALU_IDB: vm->reg_R = vm->reg_B; break;
        case ALU_OCA: vm->reg_R = ~vm->reg_A; break;
        case ALU_OCB: vm->reg_R = ~vm->reg_B; break;
        case ALU_SLA: vm->reg_R = vm->reg_A << 1; break;
        case ALU_SLB: vm->reg_R = vm->reg_B << 1; break;
        case ALU_SRA: vm->reg_R = vm->reg_A >> 1; break;
        case ALU_SRB: vm->reg_R = vm->reg_B >> 1; break;
        case ALU_AND: vm->reg_R = vm->reg_A & vm->reg_B; break;
        case ALU_IOR: vm->reg_R = vm->reg_A | vm->reg_B; break;
        case ALU_EOR: vm->reg_R = vm->reg_A ^ vm->reg_B; break;
        case ALU_ADD: vm->reg_R = vm->reg_A + vm->reg_B; break;
        case ALU_CAR: vm->reg_R = (int) vm->reg_A + (int) vm->reg_B > 255 ? 1 : 0; break;
        case ALU_ALB: vm->reg_R =  (vm->reg_A < vm->reg_B) ? 255 : 0; break;
        case ALU_AEB: vm->reg_R =  (vm->reg_A == vm->reg_B) ? 255 : 0; break;
        case ALU_AGB: vm->reg_R =  (vm->reg_A > vm->reg_B) ? 255 : 0; break;
    }
}

void
myth_SYS( Myth_vm *vm, uint8_t op)
{
      switch (op & 7)
      {
          case SYS_NOP: quitf = 1; break;
          case SYS_SSI: shift_in(vm); break;
          case SYS_SSO: shift_out(vm); break;
          case SYS_SCL: vm->ser_clock = 0; break;
          case SYS_SCH: vm->ser_clock = 1; break;
          case SYS_RDY: vm->par_ready = 1; break;
          case SYS_NEW: vm->reg_L -= 1; break;
          case SYS_OLD: vm->reg_L += 1; break;
      }
}

void
myth_TRAP( Myth_vm *vm, uint8_t addr)
{
    vm->reg_O = vm->reg_PC;
    vm->reg_PC = 0;
    vm->reg_D = vm->reg_C;
    vm->reg_C = addr;
}

void
myth_parallel_out( Myth_vm *vm, uint8_t val)
{
    vm->reg_POR = val;
    vm->par_ready=1;
    quitf = 1;
}


uint8_t
myth_fetch( Myth_vm *vm)
{
    uint16_t caddr;
    uint8_t t;
    t = vm->reg_PC++;
    caddr = t&128 ? vm->reg_G : vm->reg_C;
    caddr = 128 * caddr + (t&127);
    return vm->ram[caddr];
}

void
myth_jump( Myth_vm *vm, uint8_t val)
{
    vm->reg_BIO = vm->reg_PC;
    vm->reg_PC = val;
}

void
myth_write_reg( Myth_vm *vm, uint8_t selector, uint8_t val)
{
    uint16_t daddr;
    uint8_t t;
    switch (selector & 15)
    {
        case RHS_G: vm->reg_G = val; break;
        case RHS_M:
            t = vm->reg_xMx;
            daddr = t&128 ? vm->reg_L : vm->reg_D;
            daddr = 128 * daddr + (t&127);
            vm->ram[daddr] = val; break;
            break;
        case RHS_D: vm->reg_D = val; break;
        case RHS_O: vm->reg_O = val; break;
        case RHS_R: vm->reg_R  = val; break;
        case RHS_I: vm->reg_I = val; break;
        case RHS_S: vm->reg_SOR = val; break;
        case RHS_P: myth_parallel_out(vm, val); break;
        case RHS_E: vm->reg_E = val; break;
        case RHS_X:
            vm->reg_I--;
            if (vm->reg_I) myth_jump(vm, val);
            break;
        case RHS_J:
            myth_jump(vm, val);
            break;
        case RHS_T:
            if (vm->reg_R) myth_jump(vm, val);
            break;
        case RHS_F:
            if (!vm->reg_R) myth_jump(vm, val);
            break;
        case RHS_C: myth_TRAP(vm, val); break;
        case RHS_A: myth_set_A(vm, val); break;
        case RHS_B: myth_set_B(vm, val); break;
    }
}

uint8_t
myth_read_reg( Myth_vm *vm, uint8_t selector)
{
    uint16_t daddr;
    uint8_t t, val;
    switch (selector & 7)
    {
        case LHS_N: return myth_fetch(vm);
        case LHS_M:
            t = vm->reg_xMx;
            daddr = t&128 ? vm->reg_L : vm->reg_D;
            daddr = 128 * daddr + (t&127);
            return val = vm->ram[daddr];
        case LHS_D: return vm->reg_D;
        case LHS_O: return vm->reg_O;
        case LHS_R: return vm->reg_R;
        case LHS_I: return vm->reg_I;
        case LHS_S: vm->par_ready = 1; return vm->reg_SIR; 
        case LHS_P: return vm->reg_PIR;
        default: return 0; // Warning dummy
    }
}

void
myth_PAIR( Myth_vm *vm, uint8_t src, uint8_t dst)
{
    src &= 7;
    dst &= 15;
    if ((src == LHS_N) && (dst == RHS_M)) { // Scrounge RET
        vm->reg_PC = vm->reg_O;
        vm->reg_C = vm->reg_D;
    }
    else
    if ((src==LHS_M) && (dst==RHS_M)) vm->reg_L = 255;
    else if ((src==LHS_O) && (dst==RHS_C)) vm->reg_R = vm->reg_C;
    else if ((src==LHS_D) && (dst==RHS_C)) vm->reg_R = vm->reg_BIO;
    else if (src!=0 && src==dst);
    else myth_write_reg(vm, dst, myth_read_reg(vm, src));
}


void
myth_GETPUT( Myth_vm *vm, uint8_t bits)
{
    uint8_t guide = bits & 3;
    uint8_t gp_offs = (bits >> 4) & 3;
    uint16_t addr;

    #define PUTBIT bits & 8
    #define LOCALBIT bits & 4

    // GETPUT locations located at end of page
    if (LOCALBIT) addr = 128*(vm->reg_L+1) - 4 + gp_offs; // Local page
    else addr = 128*(vm->reg_G+1) - 4 + gp_offs; // Global page

    switch(guide) {
        case 0: if (PUTBIT) vm->ram[addr] = vm->reg_A;
                else myth_set_A(vm, vm->ram[addr]);
                break;

        case 1: if (PUTBIT) vm->ram[addr] = vm->reg_B;
                else myth_set_B(vm, vm->ram[addr]);
                break;

        case 2: if (PUTBIT) vm->ram[addr] = vm->reg_R;
                else vm->reg_R = vm->ram[addr];
                break;

        case 3: if (PUTBIT) vm->ram[addr] = vm->reg_I;
                else vm->reg_I = vm->ram[addr];
                break;
    }
}




void
myth_decode( struct myth_vm* vm,  uint8_t opcode)
{
    uint8_t dst = opcode & 15;
    uint8_t src = (opcode >> 4) & 7;
    uint8_t addend;

    //printf("%02X:%3s  ", opcode, myth_mnemonics[opcode]);

    if (opcode & 128) { myth_PAIR(vm, src, dst); return; }
    if (opcode & 64) { myth_GETPUT(vm, opcode & 63); return; }
    if (opcode & 32) { myth_TRAP(vm, opcode & 31); return; }
    if (opcode & 16) { myth_ALU(vm, dst); return; }
    if (opcode & 8) {
        switch(dst&7) {
            case 0: addend = 4; break;
            case 1: addend = 1; break;
            case 2: addend = 2; break;
            case 3: addend = 3; break;
           default: addend = (uint8_t) (0xF8 | dst&7);
        }
        vm->reg_R = vm->reg_R + addend;
        return;
    }
    myth_SYS(vm, opcode & 7);
}

void
myth_reset_warm( struct myth_vm* vm)
{
    vm->reg_L = 255;
    vm->reg_G = 255;
    vm->ser_clock = 0;
    vm->par_ready = 1;
    vm->reg_C = 0;
    vm->reg_PC = 0;
}

void
myth_reset_cold( struct myth_vm* vm)
{
    myth_reset_warm(vm);

    for (int i=0; i<16; i++) {
        vm->E_posedge[i] = NULL;
        vm->E_negedge[i] = NULL;
    }
}

void
myth_set_E( Myth_vm *vm, uint8_t val)
{
    uint8_t ldevice_old = (vm->reg_E & 15);
    uint8_t hdevice_old = (vm->reg_E >> 4);
    uint8_t ldevice = (val & 15);
    uint8_t hdevice = (val >> 4);

    vm->reg_E = val;

    if (ldevice != ldevice_old) {
        vm->E_posedge[ldevice](vm);
        vm->E_negedge[ldevice_old](vm);
    }

    if (hdevice != hdevice_old) {
        vm->E_posedge[hdevice](vm);
        vm->E_negedge[hdevice_old](vm);
    }
}



uint8_t
myth_step( struct myth_vm* vm, unsigned n)
{
    uint8_t opcode = myth_fetch( vm);
    myth_decode( vm, opcode);
    return opcode;
}



int
myth_rdimg( Myth_vm *vm, char *fname)
{
    FILE *f;
    f = fopen(fname, "r");
    if (f == NULL) return -1;
    fread(vm, 1,
        sizeof(struct myth_vm)
         - 2 * sizeof(void (*[16])(void)),
    f);
    fclose(f);
    return 0;
}

int
myth_wrimg( Myth_vm *vm, char *fname)
{
    FILE *f;
    f = fopen(fname, "rb+");
    if (f == NULL) return -1;
    if (f == NULL) return -1;
    fread(vm, 1,
        sizeof(struct myth_vm)
         - 2 * sizeof(void (*[16])(void)),
    f);
    fclose(f);
    return 0;
}










#define MAX_CYCLES 1000
int
main( int argc, char **argv)
{
    unsigned i;
    char *fname;
    struct myth_vm vm;
    loggingf = 1;

    printf("\n/* Sonne Microcontroller rev. Myth\n");
    printf("   Virtual Machine");
    printf("  Jan-2024 Michael Mangelsdorf\n");
    printf("  Copyr. <mim@ok-schalter.de>\n");
    printf("*/\n\n");

    if (argc<2){
        fname = "myth.obj";
        printf("Missing work file name, using default '%s'\n", fname);
        //exit(-1);
    } else fname = argv[1];
    
    if (myth_rdimg(&vm, fname)){
       printf("File '%s' input error\n", argv[1]);
       exit(-1);
    }
    else {
        myth_reset_cold(&vm);
        for (i=0; i<MAX_CYCLES; i++) {
            myth_decode(&vm, myth_fetch(&vm));

         //   printf("A=%02X B=%02X R=%02X C=%02X PC=%02X D=%02X W=%02X L=%02X I=%02X  L0:%02X L1:%02X L2:%02X L3:%02X\n",
         //       vm.reg_A, vm.reg_B, vm.reg_R, vm.reg_C, vm.reg_PC, vm.reg_D,
         //       vm.reg_R, vm.reg_L, vm.reg_I, vm.ram[128*(vm.reg_L+1) - 4 + 0], vm.ram[128*(vm.reg_L+1) - 4 + 1],
         //       vm.ram[128*(vm.reg_L+1) - 4 + 2], vm.ram[128*(vm.reg_L+1) - 4 + 3]);

            if (quitf) break;
        }
    }
    printf("Ax%02X Bx%02X\n", vm.reg_A, vm.reg_B);

    if (myth_wrimg(&vm, fname)){
       printf("File '%s' output error\n", argv[1]);
       exit(-1);
    } else printf("Saved after %d cycles\n", i);
}



