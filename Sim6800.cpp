// Sim6800.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "14021345"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.


#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   500



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer[MAX_BUFFER_SIZE];

char hex_file[MAX_BUFFER_SIZE];
char trc_file[MAX_BUFFER_SIZE];




//////////////////////////
// Intel 6800 Registers //
//////////////////////////

#define REGISTER_A      0
#define REGISTER_B      1

#define FLAG_N  0x08
#define FLAG_Z  0x04
#define FLAG_H  0x20
#define FLAG_I  0x10
#define FLAG_V  0x02
#define FLAG_C  0x01

BYTE Registers[2];
BYTE Flags;
WORD IndexRegister;
WORD ProgramCounter;
WORD StackPointer;

////////////
// Memory //
////////////

#define K_1                     1024
#define MEMORY_SIZE     K_1

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x00FD
#define TEST_ADDRESS_2  0x00FE
#define TEST_ADDRESS_3  0x00FF
#define TEST_ADDRESS_4  0x0100
#define TEST_ADDRESS_5  0x0101

///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;

///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][12] =
{
	"Illegal    ",
	"NOP        ",
	"Illegal    ",
	"Illegal    ",
	"Illegal    ",
	"Illegal    ",
	"TAP        ",
	"TPA        ",
	"INX        ",
	"DEX        ",
	"CLV        ",
	"SEV        ",
	"CLC        ",
	"SEC        ",
	"CLI        ",
	"SEI        ",

	"SBA        ",
	"CBA        ",
	"Illegal    ",
	"Illegal    ",
	"Illegal    ",
	"Illegal    ",
	"TAB        ",
	"TBA        ",
	"Illegal    ",
	"DAA        ",
	"Illegal    ",
	"ABA        ",
	"Illegal    ",
	"Illegal    ",
	"Illegal    ",
	"Illegal    ",

	"BRA        ",
	"Illegal    ",
	"BHI        ",
	"BLS        ",
	"BCC        ",
	"BCS        ",
	"BNE        ",
	"BEQ        ",
	"BVC        ",
	"BVS        ",
	"BPL        ",
	"BMI        ",
	"BGE        ",
	"BLT        ",
	"BGT        ",
	"BLE        ",

	"TSX        ",
	"INS        ",
	"PULA       ",
	"PULB       ",
	"DES        ",
	"TXS        ",
	"PSHA       ",
	"PSHB       ",
	"Illegal    ",
	"RTS        ",
	"Illegal    ",
	"RTI        ",
	"Illegal    ",
	"Illegal    ",
	"WAI        ",
	"SWI        ",

	"NEGA       ",
	"Illegal    ",
	"Illegal    ",
	"COMA       ",
	"LSRA       ",
	"Illegal    ",
	"RORA       ",
	"ASRA       ",
	"ASLA       ",
	"ROLA       ",
	"DECA       ",
	"Illegal    ",
	"INCA       ",
	"TSTA       ",
	"Illegal    ",
	"CLRA       ",

	"NEGB       ",
	"Illegal    ",
	"Illegal    ",
	"COMB       ",
	"LSRB       ",
	"Illegal    ",
	"RORB       ",
	"ASRB       ",
	"ASLB       ",
	"ROLB       ",
	"DECB       ",
	"Illegal    ",
	"INCB       ",
	"TSTB       ",
	"Illegal    ",
	"CLRB       ",

	"NEG d,X    ",
	"Illegal    ",
	"Illegal    ",
	"COM d,X    ",
	"LSR d,X    ",
	"Illegal    ",
	"ROR d,X    ",
	"ASR d,X    ",
	"ASL d,X    ",
	"ROL d,X    ",
	"DEC d,X    ",
	"Illegal    ",
	"INC d,X    ",
	"TST d,X    ",
	"JMP d,X    ",
	"CLR d,X    ",

	"NEG w      ",
	"Illegal    ",
	"Illegal    ",
	"COM w      ",
	"LSR w      ",
	"Illegal    ",
	"ROR w      ",
	"ASR w      ",
	"ASL w      ",
	"ROL w      ",
	"DEC w      ",
	"Illegal    ",
	"INC w      ",
	"TST w      ",
	"JMP w      ",
	"CLR w      ",

	"SUBA #d    ",
	"CMPA #d    ",
	"SBCA #d    ",
	"Illegal    ",
	"ANDA #d    ",
	"BITA #d    ",
	"LDAA #d    ",
	"Illegal    ",
	"EORA #d    ",
	"ADCA #d    ",
	"ORAA #d    ",
	"ADDA #d    ",
	"CPX #w     ",
	"BSR d      ",
	"LDS #w     ",
	"Illegal    ",

	"SUBA d     ",
	"CMPA d     ",
	"SBCA d     ",
	"Illegal    ",
	"ANDA d     ",
	"BITA d     ",
	"LDAA d     ",
	"STAA d     ",
	"EORA d     ",
	"ADCA d     ",
	"ORAA d     ",
	"ADDA d     ",
	"CPX d      ",
	"Illegal    ",
	"LDS d      ",
	"STS d      ",

	"SUBA d,X   ",
	"CMPA d,X   ",
	"SBCA d,X   ",
	"Illegal    ",
	"ANDA d,X   ",
	"BITA d,X   ",
	"LDAA d,X   ",
	"STAA d,X   ",
	"EORA d,X   ",
	"ADCA d,X   ",
	"ORAA d,X   ",
	"ADDA d,X   ",
	"CPX d,X    ",
	"JSR d,X    ",
	"LDS d,X    ",
	"STS d,X    ",

	"SUBA w     ",
	"CMPA w     ",
	"SBCA w     ",
	"Illegal    ",
	"ANDA w     ",
	"BITA w     ",
	"LDAA w     ",
	"STAA w     ",
	"EORA w     ",
	"ADCA w     ",
	"ORAA w     ",
	"ADDA w     ",
	"CPX w      ",
	"JSR w      ",
	"LDS w      ",
	"STS w      ",

	"SUBB #d    ",
	"CMPB #d    ",
	"SBCB #d    ",
	"Illegal    ",
	"ANDB #d    ",
	"BITB #d    ",
	"LDAB #d    ",
	"Illegal    ",
	"EORB #d    ",
	"ADCB #d    ",
	"ORAB #d    ",
	"ADDB #d    ",
	"Illegal    ",
	"Illegal    ",
	"LDX #w     ",
	"Illegal    ",

	"SUBB d     ",
	"CMPB d     ",
	"SBCB d     ",
	"Illegal    ",
	"ANDB d     ",
	"BITB d     ",
	"LDAB d     ",
	"STAB d     ",
	"EORB d     ",
	"ADCB d     ",
	"ORAB d     ",
	"ADDB d     ",
	"Illegal    ",
	"Illegal    ",
	"LDX d      ",
	"STX d      ",

	"SUBB d,X   ",
	"CMPB d,X   ",
	"SBCB d,X   ",
	"Illegal    ",
	"ANDB d,X   ",
	"BITB d,X   ",
	"LDAB d,X   ",
	"STAB d,X   ",
	"EORB d,X   ",
	"ADCB d,X   ",
	"ORAB d,X   ",
	"ADDB d,X   ",
	"Illegal   ",
	"Illegal    ",
	"LDX d,X    ",
	"STX d,X    ",

	"SUBB w     ",
	"CMPB w     ",
	"SBCB w     ",
	"Illegal    ",
	"ANDB w     ",
	"BITB w     ",
	"LDAB w     ",
	"STAB w     ",
	"EORB w     ",
	"ADCB w     ",
	"ORAB w     ",
	"ADDB w     ",
	"Illegal    ",
	"Illegal    ",
	"LDX w      ",
	"STX w      "
};







////////////////////////////////////////////////////////////////////////////////
//                      Intel 6800 Simulator/Emulator (Start)                 //
////////////////////////////////////////////////////////////////////////////////


BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}


// Add any instruction implementing routines here...



void set_flag_z(BYTE inReg)
{
	BYTE reg;

	reg = inReg;

	if (reg == 0)                   // set to zero
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

void set_flag_zword(WORD inReg) //zero flag word
{
	WORD reg;
	reg = inReg;
	if (reg == 0)                   // set to zero
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}


void set_carry_flag(BYTE inReg)
{
	WORD temp_word;

	temp_word = inReg;

	if (temp_word >= 0x100)
	{
		//Set Carry Flag
		Flags = Flags | FLAG_C;
	}
	else
	{
		//Clear Carry Flag
		Flags = Flags & (0xFF - FLAG_C);
	}
}

void set_flag_n(BYTE inReg)
{
	BYTE reg;

	reg = inReg;

	if ((reg & 0x80) != 0)   //msbit set
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

void set_flag_nword(BYTE inReg)
{
	WORD reg;

	reg = inReg;

	if ((reg & 0x8000) != 0)
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

void set_flag_v(BYTE in1, BYTE in2, BYTE out1)
{
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;

	reg1in = in1;
	reg2in = in2;
	regOut = out1;

	//code for overflow flag given in PAL session

	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80))    //overflow
		|| (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80)))
	{
		Flags = Flags | FLAG_V;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_V);
	}

}


void Group_1_Dual_Operand_Instructions(BYTE opcode)
{
	BYTE regA;
	BYTE regB;
	BYTE lb;
	BYTE hb;
	BYTE byte_address;
	WORD wresult;
	WORD wdata;
	WORD hi_address, lo_address;
	WORD address;
	WORD temp_word;
	WORD offset;



	switch (opcode)
	{
	case 0x86:  // LDAA immediate
		Registers[REGISTER_A] = fetch();
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xA6:   //LDAA indexed
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Registers[REGISTER_A] = Memory[address];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xB6:  // LDAA extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Registers[REGISTER_A] = Memory[address];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0x96:  // LDAA direct
		address = (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Registers[REGISTER_A] = Memory[address];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;



	case 0x97:  // STAA direct
		address = (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Memory[address] = Registers[REGISTER_A];
		}
		else{}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xA7: //STAA indexed
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Memory[address] = Registers[REGISTER_A];
		}
		else {}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xB7:  //StAA extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Memory[address] = Registers[REGISTER_A];
		}
		else{}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xC6:  // LDAB immediate
		Registers[REGISTER_B] = fetch();
		//flags
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xD6: // LDAB direct
		address = (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Registers[REGISTER_B] = Memory[address];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xE6:  // LDAB indexed
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Registers[REGISTER_B] = Memory[address];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xF6:  //LDAB extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Registers[REGISTER_B] = Memory[address];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xD7:  // STAB direct
		address = (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Memory[address] = Registers[REGISTER_B];
		}
		else{}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xE7:  //STAB indexed
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Memory[address] = Registers[REGISTER_B];
		}
		else {}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xF7:  // STAB extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		if ((address >= 0) && (address<MEMORY_SIZE))
		{
			Memory[address] = Registers[REGISTER_B];
		}
		else{}
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0xCE: // LDX immediate
		IndexRegister = (((WORD)fetch()) << 8) + fetch();
		temp_word = (WORD)IndexRegister;
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xDE:  // LDX direct
		address = (WORD)fetch();
		temp_word = (WORD)IndexRegister;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			IndexRegister = (((WORD)Memory[address]) << 8) + (WORD)Memory[address + 1];
		}
		else
		{
			IndexRegister = 0;
		}
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xEE:  //LDX indexed
		address = IndexRegister + (WORD)fetch();
		temp_word = (WORD)IndexRegister;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			IndexRegister = (((WORD)Memory[address]) << 8) + (WORD)Memory[address + 1];
		}
		else
		{
			IndexRegister = 0;
		}
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xFE: // LDX extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		temp_word = (WORD)IndexRegister;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			IndexRegister = (((WORD)Memory[address]) << 8) + (WORD)Memory[address + 1];
		}
		else
		{
			IndexRegister = 0;
		}
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xDF:  //  STX direct
		address = (WORD)fetch();
		temp_word = (WORD)IndexRegister;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			Memory[address] = (BYTE)((IndexRegister >> 8) & 0xFF);
			Memory[address + 1] = (BYTE)((IndexRegister >> 8) & 0xFF);
		}
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xEF: // STX Indexed
		address = IndexRegister + (WORD)fetch();
		temp_word = (WORD)IndexRegister;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			Memory[address] = (BYTE)((IndexRegister >> 8) & 0xFF);
			Memory[address + 1] = (BYTE)((IndexRegister >> 8) & 0xFF);
		}
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xFF:  // STX extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		temp_word = (WORD)IndexRegister;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			Memory[address] = (BYTE)((IndexRegister >> 8) & 0xFF);
			Memory[address + 1] = (BYTE)((IndexRegister >> 8) & 0xFF);
		}
		//flags
		if (IndexRegister < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x8E: //LDS immediate
		StackPointer = (((WORD)fetch()) << 8) + fetch();
		temp_word = (WORD)StackPointer;
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x9E: //LDS Direct
		address = (WORD)fetch();
		temp_word = (WORD)StackPointer;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			StackPointer = (((WORD)Memory[address]) << 8) + (WORD)Memory[address + 1];
		}
		else
		{
			StackPointer = 0;
		}
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xAE: //LDS indexed
		address = IndexRegister + (WORD)fetch();
		temp_word = (WORD)StackPointer;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			StackPointer = (((WORD)Memory[address]) << 8) + (WORD)Memory[address + 1];
		}
		else
		{
			StackPointer = 0;
		}
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xBE: //LDS extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		temp_word = (WORD)StackPointer;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			StackPointer = (((WORD)Memory[address]) << 8) + (WORD)Memory[address + 1];
		}
		else
		{
			StackPointer = 0;
		}
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x9F: // STS direct
		address = (WORD)fetch();
		temp_word = (WORD)StackPointer;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			Memory[address] = (BYTE)((StackPointer >> 8) & 0xFF);
			Memory[address + 1] = (BYTE)((StackPointer >> 8) & 0xFF);
		}
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xAf: //STS indexed
		address = IndexRegister + (WORD)fetch();
		temp_word = (WORD)StackPointer;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			Memory[address] = (BYTE)((StackPointer >> 8) & 0xFF);
			Memory[address + 1] = (BYTE)((StackPointer >> 8) & 0xFF);
		}
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0xBF: //STS extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		temp_word = (WORD)StackPointer;
		if ((address >= 0) && (address<(MEMORY_SIZE - 1)))
		{
			Memory[address] = (BYTE)((StackPointer >> 8) & 0xFF);
			Memory[address + 1] = (BYTE)((StackPointer >> 8) & 0xFF);
		}
		//flags
		if (StackPointer < 0x00)
		{
			Flags = Flags | FLAG_N;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_N);
		}
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_z((BYTE)temp_word);
		break;

	case 0x8B: // ADDA immediate
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;

		break;

	case 0x9B: //ADDA direct
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xAB: //ADDA Index
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xBB: //ADDA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xCB: //ADDB Immediate
		regB = Registers[REGISTER_B];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xDB: //ADDB Direct
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xEB: //ADDB Index
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xFB: //ADDB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;



	case 0x89: //ADCA immediate
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;

		break;

	case 0x99: //ADCA Direct
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA9: //ADCA Index
		address = IndexRegister + (WORD)fetch();
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB9: //ADCA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regA = Registers[REGISTER_A];
		address = (hb << 8) + lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC9: //ADCB Immediate
		regB = Registers[REGISTER_B];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xD9: //ADCB Direct
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE9: //ADCB Index
		address = IndexRegister + (WORD)fetch();
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF9: //ADCB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regB = Registers[REGISTER_B];
		address = (hb << 8) + lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;



	case 0x81:  //CMPA immediate
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
	
		
		break;

	case 0x91: //CMPA Direct
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA1: //CMPA Index
		address = IndexRegister + (WORD)fetch();
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB1: //CMPA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regA = Registers[REGISTER_A];
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[(WORD)address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC1: //CMPB Immediate
		regB = Registers[REGISTER_B];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		break;

	case 0xD1: //CMPB Direct
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE1: //CMPB Index
		address = IndexRegister + (WORD)fetch();
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF1: //CMPB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regB = Registers[REGISTER_B];
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[(WORD)address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xAD: //JSR indexed
		address = IndexRegister + (WORD)fetch();
		hb = Memory[address];
		lb = Memory[address + 1];
		address = ((WORD)hb << 8) + (WORD)lb;
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			ProgramCounter = address;
		}

		break;

	case 0xBD: //JSR extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			ProgramCounter = address;
		}

		break;

	case 0x8D: //BSR
		lb = fetch();
		offset = (WORD)lb;
		if ((offset & 0x80) != 0) // need to sign extend
		{
			offset = offset + 0xFF00;
		}
		address = ProgramCounter + offset;
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
		}
		ProgramCounter = address;
		break;

	case 0x84: //ANDA Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x94: //ANDA Direct
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA4: //ANDA Index
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB4: //ANDA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC4: //ANDB Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xD4: //ANDB Direct
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE4: //ANDB Indexed
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF4: //ANDB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x85: //BITA Immediate
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x95: //BITA Direct
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xA5: //BITA Index
		address = IndexRegister & (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xB5: //BITA extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xC5: //BITB Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xD5: //BITB Direct
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xE5: //BITB Index
		address = IndexRegister & (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xF5: //BITB Extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x80:      //SUBA Immediate
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;


	case 0x90: //SUBA Direct
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA0: //SUBA Index
		regA = Registers[REGISTER_A];
		address = IndexRegister - (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB0: //SUBA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) - lb;
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC0: //SUBB Immediate
		regB = Registers[REGISTER_B];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xD0: //SUBB Direct
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE0: //SUBB Index
		regB = Registers[REGISTER_B];
		address = IndexRegister - (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF0: //SUBB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) - lb;
		regA = Registers[REGISTER_B];
		address = (WORD)fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x82: //SBCA Immediate
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x92: //SBCA Direct
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;


	case 0xA2: //SBCA Index
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB2: //SBCA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regA = Registers[REGISTER_A];
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC2: //SBCB Immediate
		regB = Registers[REGISTER_B];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		//flags
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xD2: //SBCB Direct
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE2: //SBCB Index
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF2: //SBCB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		regB = Registers[REGISTER_B];
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		if ((Flags & FLAG_C) != 0)
		{
			temp_word--;
		}
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x8C: //CPX Immediate
		wdata = (((WORD)fetch()) << 8) + fetch();
		wresult = IndexRegister - wdata;
		//flags
		set_flag_z(wresult);
		set_flag_n(wresult);
		set_flag_v((BYTE)IndexRegister >> 8, (BYTE)wdata, (BYTE)wresult >> 8);
		break;

	case 0x9C: //CPX Direct
		address = fetch();
		hb = Memory[address];
		lb = Memory[address + 1];
		wdata = (((WORD)hb) << 8) + lb;
		wresult = IndexRegister - wdata;
		//flags
		set_flag_z(wresult);
		set_flag_n(wresult);
		set_flag_v((BYTE)IndexRegister >> 8, (BYTE)wdata, (BYTE)wresult >> 8);
		break;

	case 0xAC: //CPX Index
		address = IndexRegister + (WORD)fetch;
		hb = Memory[address];
		lb = Memory[address + 1];
		wdata = (((WORD)hb) << 8) + lb;
		wresult = IndexRegister - wdata;
		//flags
		set_flag_z(wresult);
		set_flag_n(wresult);
		set_flag_v((BYTE)IndexRegister >> 8, (BYTE)wdata, (BYTE)wresult >> 8);
		break;

	case 0xBC: //CPX Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		hb = Memory[address];
		lb = Memory[address + 1];
		wdata = (((WORD)hb) << 8) + lb;
		wresult = IndexRegister - wdata;
		//flags
		set_flag_z(wresult);
		set_flag_n(wresult);
		set_flag_v((BYTE)IndexRegister >> 8, (BYTE)wdata, (BYTE)wresult >> 8);
		break;

	case 0x88: //EORA Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)lb;
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x98: //EORA Direct
		address = (WORD)fetch();
		if ((address >= 0) ^ (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xA8: //EORA Index
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) ^ (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB8: //EORA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		address = (WORD)fetch();
		if ((address >= 0) ^ (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC8: //EORB Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] ^ (WORD)lb;
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xD8: //EORB Direct
		address = (WORD)fetch();
		if ((address >= 0) ^ (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] ^ (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE8: //EORB Index
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) ^ (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] ^ (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF8: //EORB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		address = (WORD)fetch();
		if ((address >= 0) ^ (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] ^ (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0x8A: //ORAA Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] | (WORD)lb;
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x9A: //ORAA Direct
		address = (WORD)fetch();
		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] | (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xAA: //ORAA Index
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] | (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xBA: //ORAA Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		address = (WORD)fetch();
		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] | (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xCA: //ORAB Immediate
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] | (WORD)lb;
		//flags
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xDA: //ORAB Direct
		address = (WORD)fetch();
		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] | (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xEA: //ORAB Index
		address = IndexRegister + (WORD)fetch();
		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] | (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xFA: //ORAB Extended
		hb = (WORD)fetch();
		lb = (WORD)fetch();
		address = (hb << 8) + lb;
		address = (WORD)fetch();
		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] | (WORD)lb;
		//flags
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	default:
		break;
	}
}


void Group_2_Single_Operand_Instructions(BYTE opcode)
{
	BYTE regA;
	BYTE regB;
	BYTE lb;
	BYTE hb;
	BYTE byte_address;
	BYTE saved_flags;
	WORD wdata;
	WORD hi_address, lo_address;
	WORD address;
	WORD temp_word;
	WORD offset;

	switch (opcode)
	{
	case  0x6E: //JMP indexed
		address = IndexRegister + (WORD)fetch();
		hb = Memory[address];
		lb = Memory[address + 1];
		address = ((WORD)hb << 8) + (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			ProgramCounter = address;
		}
		break;

	case 0x7E: //JMP Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			ProgramCounter = address;
		}
		break;

	case 0x4C: //INCA
		Registers[REGISTER_A] = Registers[REGISTER_A] + 1;
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		if (Registers[REGISTER_A] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x5C: //INCB
		Registers[REGISTER_B] = Registers[REGISTER_B] + 1;
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Registers[REGISTER_B];
		if (Registers[REGISTER_B] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x6C: //INC Indexed
		address = IndexRegister + (WORD)fetch();
		Memory[address]++;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		set_flag_n(Memory[address]);
		set_flag_z(Memory[address]);
		if (Memory[address] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x7C: //INC Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		Memory[address]++;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		set_flag_n(Memory[address]);
		set_flag_z(Memory[address]);
		if (address == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x6A: //DEC Indexed
		address = IndexRegister + (WORD)fetch();
		Memory[address]--;
		set_flag_n((BYTE)address);
		set_flag_z((BYTE)address);
		if (address == 0x7F)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x7A: //DEC Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) - (WORD)lb;
		Memory[address]--;
		set_flag_n((BYTE)address);
		set_flag_z((BYTE)address);
		if (address == 0x7F)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x4A: //DECA
		Registers[REGISTER_A]--;
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Registers[REGISTER_A];
		if (Registers[REGISTER_A] == 0x7F)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x5A: //DECB
		Registers[REGISTER_B]--;
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Registers[REGISTER_B];
		if (Registers[REGISTER_B] == 0x7F)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x4F: //CLRA Inherent
		Registers[REGISTER_A] = 0;
		Flags = Flags & (0xFF - FLAG_N);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		set_flag_z((BYTE)Registers[REGISTER_A]);
		break;

	case 0x5F: //CLRB Inherent
		Registers[REGISTER_B] = 0;
		Flags = Flags & (0xFF - FLAG_N);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		set_flag_z((BYTE)Registers[REGISTER_B]);
		break;

	case 0x6F: //CLR Index
		address = IndexRegister + (WORD)fetch();
		Memory[address] = 0;
		//Flags
		Flags = Flags & (0xFF - FLAG_N);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		set_flag_z((BYTE)address);
		break;

	case 0x7F: //CLR Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		Memory[address] = 0;
		//Flags
		Flags = Flags & (0xFF - FLAG_N);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		set_flag_z((BYTE)address);
		break;

	case 0x43: //COMA
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ 0xFF;
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags | FLAG_C;
		break;

	case 0x53: // COMB
		Registers[REGISTER_B] = Registers[REGISTER_B] ^ 0xFF;
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags | FLAG_C;
		break;

	case 0x63: // COM Index
		address = IndexRegister + fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			temp_word = Memory[address];
		}
		else
		{
			temp_word = 0;
		}
		temp_word = temp_word ^ 0xFF;
		Memory[address] = temp_word;
		set_flag_n(temp_word);
		set_flag_z(temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags | FLAG_C;
		break;

	case 0x73: //COM Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			temp_word = Memory[address];
		}
		else
		{
			temp_word = 0;
		}
		temp_word = Memory[address];
		temp_word = temp_word ^ 0xFF;
		Memory[address] = temp_word;
		set_flag_n(temp_word);
		set_flag_z(temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags | FLAG_C;
		break;

	case 0x60: //NEG Indexed
		address = IndexRegister + fetch();
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			temp_word = Memory[address];
		}
		else
		{
			temp_word = 0;
		}
		temp_word = (temp_word ^ 0xFF) + 1;
		Memory[address] = temp_word;
		set_flag_n(temp_word);
		set_flag_z(temp_word);
		if (Flags == 0x00)
		{
			Flags | FLAG_C;
		}
		if ((Flags & 0x80) == 0x80)
		{
			Flags | FLAG_V;
		}
		break;

	case 0x70: //NEG Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			temp_word = Memory[address];
		}
		else
		{
			temp_word = 0;
		}
		temp_word = Memory[address];
		temp_word = (temp_word ^ 0xFF) + 1;
		Memory[address] = temp_word;
		set_flag_n(temp_word);
		set_flag_z(temp_word);
		if (Flags == 0x00)
		{
			Flags | FLAG_C;
		}
		if ((Flags & 0x80) == 0x80)
		{
			Flags | FLAG_V;
		}
		break;

	case 0x40: //NEGA
		Registers[REGISTER_A] = (Registers[REGISTER_A] ^ 0xFF) + 1;
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		if (Flags == 0x00)
		{
			Flags | FLAG_C;
		}
		if ((Flags & 0x80) == 0x80)
		{
			Flags | FLAG_V;
		}
		break;

	case 0x50: // NEGB
		Registers[REGISTER_B] = (Registers[REGISTER_B] ^ 0xFF) + 1;
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		if (Flags == 0x00)
		{
			Flags | FLAG_C;
		}
		if ((Flags & 0x80) == 0x80)
		{
			Flags | FLAG_V;
		}
		break;

	case 0x44: //LSRA
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x01) == 0x01) //carry
		{
			Flags = Flags | FLAG_C; //setting the carry flag.
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		if (Registers[REGISTER_A] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x54: //LSRB
		saved_flags = Flags;
		if ((Registers[REGISTER_B] & 0x01) == 0x01) //carry
		{
			Flags = Flags | FLAG_C; //setting the carry flag.
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_B] = (Registers[REGISTER_B] >> 1) & 0x7F;
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		if (Registers[REGISTER_B] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x64: //LSR Index
		saved_flags = Flags;
		address = IndexRegister + (WORD)fetch();
		if ((Memory[address] & 0x01) == 0x01) //carry
		{
			Flags = Flags | FLAG_C; //setting the carry flag.
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] >> 1) & 0x7F;
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		if (Memory[address] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x74: //LSR Extended
		saved_flags = Flags;
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((Memory[address] & 0x01) == 0x01) //carry
		{
			Flags = Flags | FLAG_C; //setting the carry flag.
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] >> 1) & 0x7F;
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		if (Memory[address] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x48: //ASLA
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
		if (((Registers[REGISTER_A] & 0x80) == 0x80) ^ ((Flags &FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x58: //ASLB
		saved_flags = Flags;
		if ((Registers[REGISTER_B] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_B] = (Registers[REGISTER_B] << 1) & 0xFE;
		if (((Registers[REGISTER_B] & 0x80) == 0x80) ^ ((Flags &FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_B] = Registers[REGISTER_B] | 0x01;
		}
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

	case 0x68: //ASL Index
		saved_flags = Flags;
		address = IndexRegister + (WORD)fetch();
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x78: //ASL Extended
		saved_flags = Flags;
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x47: //ASRA //// needs to be changed
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0xFE;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		if (((Registers[REGISTER_A] & 0x80) == 0x80) ^ ((Flags &FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x57: //ASRB
		saved_flags = Flags;
		if ((Registers[REGISTER_B] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_B] = (Registers[REGISTER_B] >> 1) & 0xFE;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_B] = Registers[REGISTER_B] | 0x01;
		}
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		if (((Registers[REGISTER_A] & 0x80) == 0x80) ^ ((Flags &FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x67: //ASR index
		saved_flags = Flags;
		address = IndexRegister + (WORD)fetch();
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] >> 1) & 0xFE;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x77: //ASR Extended
		saved_flags = Flags;
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] >> 1) & 0xFE;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x49: //ROLA
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
		if (((Registers[REGISTER_A] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x59: //ROLB
		saved_flags = Flags;
		if ((Registers[REGISTER_B] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_B] = (Registers[REGISTER_B] << 1) & 0xFE;
		if (((Registers[REGISTER_B] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_B] = Registers[REGISTER_B] | 0x01;
		}
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

	case 0x69: //ROL Index
		saved_flags = Flags;
		address = IndexRegister + (WORD)fetch();
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x79: //ROL Extended
		saved_flags = Flags;
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x46: //RORA ///// RORS may need to be changed
		saved_flags = Flags;
		if ((Registers[REGISTER_A] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0xFE;
		if (((Registers[REGISTER_A] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		if (((Registers[REGISTER_A] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x56: //RORB
		saved_flags = Flags;
		if ((Registers[REGISTER_B] & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_B] = (Registers[REGISTER_B] >> 1) & 0xFE;

		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Registers[REGISTER_B] = Registers[REGISTER_B] | 0x01;
		}
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		if (((Registers[REGISTER_B] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x66: //ROR Index
		saved_flags = Flags;
		address = IndexRegister + (WORD)fetch();
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] >> 1) & 0xFE;

		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x76: //ROR Extended
		saved_flags = Flags;
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		if ((address & 0x80) == 0x80) //carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] >> 1) & 0xFE;
		if ((saved_flags & FLAG_C) == FLAG_C)
		{
			Memory[address] = Memory[address] | 0x01;
		}
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		if (((Memory[address] & 0x80) == 0x80) ^ ((Flags & FLAG_C) == FLAG_C))//overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		break;

	case 0x4D: //TSTA
		Registers[REGISTER_A] = Registers[REGISTER_A] - 0x00;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);

		break;

	case 0x5D: //TSTB
		Registers[REGISTER_B] = Registers[REGISTER_B] - 0x00;
		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x6D: //TST Index
		address = IndexRegister + (WORD)fetch();
		Memory[address] = Memory[address] - 0x00;
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x7D: //TST Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + lb;
		Memory[address] = Memory[address] - 0x00;
		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;


	default:
		break;
	}
}



void Group_3_TPA_TAP_NOP_Instructions(BYTE opcode)
{

	BYTE regA;
	BYTE regB;
	BYTE lb;
	BYTE hb;
	BYTE byte_address;
	WORD wdata;
	WORD hi_address, lo_address;
	WORD address;
	WORD temp_word;

	switch (opcode)
	{

	case 0x06: //TAP inher
		Flags = Registers[REGISTER_A];
		break;

	case 0x07: //TPA inher
		Registers[REGISTER_A] = Flags;
		break;

	case 0x01: // NOP
		break;


	default:
		break;
	}


}


void Group_4_Condition_Code_Instructions(BYTE opcode)
{
	BYTE regA;
	BYTE regB;
	BYTE lb;
	BYTE hb;
	BYTE byte_address;
	WORD wdata;
	WORD hi_address, lo_address;
	WORD address;
	WORD temp_word;
	WORD offset;

	switch (opcode)
	{

	case 0x0C: // CLC inher
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x0E: // CLI  inher
		Flags = Flags & (0xFF - FLAG_I);
		break;

	case 0x0A: // CLV inher
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x0D: //SEC inher
		Flags = Flags | FLAG_C;
		break;

	case 0x0F: //SEI inher
		Flags = Flags | FLAG_I;
		break;

	case 0x0B: //SEV inher
		Flags = Flags | FLAG_V;
		break;

	case 0x08: //INX
		IndexRegister++;
		if (IndexRegister == 0)
		{
			Flags = Flags | FLAG_Z;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_Z);
		}
		break;

	case 0x09: //DEX
		IndexRegister--;
		if (IndexRegister == 0)
		{
			Flags = Flags | FLAG_Z;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_Z);
		}
		break;






	default:
		break;
	}
}


void Group_5_Accumulator_Instructions(BYTE opcode)
{
	BYTE regA, lb, hb;
	WORD temp_word;
	switch (opcode)
	{

	case 0x10: //SBA
		regA = Registers[REGISTER_A];
		Registers[REGISTER_A] = Registers[REGISTER_A] - Registers[REGISTER_B];
		//flags
		set_carry_flag(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		set_flag_v(regA, Registers[REGISTER_B], Registers[REGISTER_A]);

		break;

	case 0x1B: //ABA Inher
		regA = Registers[REGISTER_A];
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_B];
		set_carry_flag(temp_word);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, Registers[REGISTER_B], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0x11: //CBA
		regA = Registers[REGISTER_A];
		temp_word = Registers[REGISTER_A] - Registers[REGISTER_B];
		//flags
		set_carry_flag(temp_word);
		set_flag_n(temp_word);
		set_flag_z(temp_word);
		set_flag_v(regA, Registers[REGISTER_B], temp_word);
		break;

	case 0x19: // DAA inherent
		temp_word = Registers[REGISTER_A];
		lb = Registers[REGISTER_A] & 0x0F;
		hb = Registers[REGISTER_A] >> 4;
		if ((lb > 9) || (Flags & FLAG_C) == FLAG_C) {
			lb = lb + 6;
		}
		if ((hb > 9) || (Flags & FLAG_C) == FLAG_C) {
			hb = hb + 6;
		}
		Registers[REGISTER_A] = lb + (hb << 4);
		temp_word = Registers[REGISTER_A];
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(lb, hb, (BYTE)temp_word);
		break;

	case 0x16: //TAB
		Registers[REGISTER_B] = Registers[REGISTER_A];
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)Registers[REGISTER_B]);
		set_flag_z((BYTE)Registers[REGISTER_B]);
		break;

	case 0x17: //TBA
		Registers[REGISTER_A] = Registers[REGISTER_B];
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)Registers[REGISTER_A]);
		set_flag_z((BYTE)Registers[REGISTER_A]);
		break;

	default:
		break;
	}
}


void Group_6_Branch_Instructions(BYTE opcode)
{
	BYTE regA;
	BYTE regB;
	BYTE lb, N, Z, V, C;
	BYTE hb;
	BYTE byte_address;
	WORD wdata;
	WORD hi_address, lo_address;
	WORD address;
	WORD temp_word;
	WORD offset;

	if ((Flags & FLAG_N) != 0)
	{
		N = 1;
	}
	else
	{
		N = 0;
	}

	if ((Flags & FLAG_Z) != 0)
	{
		Z = 1;
	}
	else
	{
		Z = 0;
	}

	if ((Flags & FLAG_V) != 0)
	{
		V = 1;
	}
	else
	{
		V = 0;
	}

	if ((Flags & FLAG_C) != 0)
	{
		C = 1;
	}
	else
	{
		C = 0;
	}

	switch (opcode)
	{


	case 0x20: //BRA Direct
		lb = fetch();
		offset = (WORD)lb;
		if ((offset & 0x80) != 0) // need to sign extend
		{
			offset = offset + 0xFF00;
		}
		address = ProgramCounter + offset;
		ProgramCounter = address;
		break;

	case 0x24: //BCC Direct
		lb = fetch();
		if ((Flags & FLAG_C) != FLAG_C)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x25: //BCS Direct
		lb = fetch();
		if ((Flags & FLAG_C) != FLAG_C)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 1)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x27: //BEQ Direct
		lb = fetch();
		if ((Flags & FLAG_Z) != FLAG_Z)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 1)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2C: //BGE Direct
		lb = fetch();
		if ((N ^V) == 0)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2E: //BGT Direct
		lb = fetch();
		if ((Z | N ^ V) == 1)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x22: //BHI Direct
		lb = fetch();
		if (C | Z == 1)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2F: //BLE Direct
		lb = fetch();
		if ((Z | N ^ V) == 1)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 1)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x23: //BLS Direct
		lb = fetch();
		if (C | Z == 1)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 1)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}

	case 0x2D: //BLT DIRECT
		lb = fetch();
		if (N ^ V == 1)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 1)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2B: //BMI Direct
		lb = fetch();
		if ((Flags & FLAG_N) != FLAG_N)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 1)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x26: //BNE Direct
		lb = fetch();
		if ((Flags & FLAG_Z) != FLAG_Z)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x28: //BVC Direct
		lb = fetch();
		if ((Flags & FLAG_V) != FLAG_V)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x29: //BVS
		lb = fetch();
		if ((Flags & FLAG_V) != FLAG_V)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2A: //BPL Direct
		lb = fetch();
		if ((Flags & FLAG_N) != FLAG_N)
		{
			offset = (WORD)lb;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;



	default:
		break;

	}
}




void Group_7_Stack_and_Index_Register_Instructions(BYTE opcode)
{
	switch (opcode)
	{

	case 0x35: // TXS Inher
		StackPointer = IndexRegister - 1;
		break;

	case 0x30: // TSX inher
		StackPointer = IndexRegister + 1;
		break;

	case 0x36: //PSHA inher
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_A];
			StackPointer--;
		}
		break;

	case 0x37: //PSHB inher
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = Registers[REGISTER_B];
			StackPointer--;
		}
		break;

	case 0x32: //PULA inher
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
		{
			StackPointer++;
			Registers[REGISTER_A] = Memory[StackPointer];

		}
		break;

	case 0x33: //PULB inher
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
		{
			StackPointer++;
			Registers[REGISTER_B] = Memory[StackPointer];

		}
		break;

	case 0x34: //DES
		StackPointer--;
		break;

	case 0x31: //INS
		StackPointer++;
		break;

	default:
		break;
	}
}


void Group_8_Interrupt_and_Subroutine_Instructions(BYTE opcode)
{
	BYTE regA;
	BYTE regB;
	BYTE lb;
	BYTE hb;
	BYTE byte_address;
	BYTE hi_prog, lo_prog, hi_index, lo_index;
	WORD wdata;
	WORD hi_address, lo_address;
	WORD address;
	WORD temp_word;
	WORD offset;


	switch (opcode)
	{
	case 0x3E:  // WAI
		halt = true;
		break;

	case 0x39: //RTS
		if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2)))
		{
			StackPointer++;
			hb = Memory[StackPointer];
			StackPointer++;
			lb = Memory[StackPointer];
			ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
		}

		break;

	case 0x3B: //RTI Inher
		address = IndexRegister + (WORD)fetch();


		if (StackPointer > IndexRegister)
		{
			StackPointer++;
			Flags = Memory[StackPointer];
			StackPointer++;
			Registers[REGISTER_B] = Memory[StackPointer];
			StackPointer++;
			Registers[REGISTER_A] = Memory[StackPointer];
			StackPointer++;
			hi_index = Memory[StackPointer];
			StackPointer++;
			lo_index = Memory[StackPointer];
			StackPointer++;
			hi_prog = Memory[StackPointer];
			StackPointer++;
			lo_prog = Memory[StackPointer];
			StackPointer++;


		}
		ProgramCounter = (WORD)((hi_prog << 8) + lo_prog);
		IndexRegister = (WORD)((hi_index << 8) + lo_index);
		break;

	case 0x3F: //SWI Inher
		address = IndexRegister + (WORD)fetch();
		hi_prog = (BYTE)ProgramCounter >> 8;
		lo_prog = (BYTE)ProgramCounter;

		hi_index = (BYTE)IndexRegister >> 8;
		lo_index = (BYTE)IndexRegister;

		if (StackPointer > IndexRegister)
		{

			Memory[StackPointer] = lo_prog;
			StackPointer--;
			Memory[StackPointer] = hi_prog;
			StackPointer--;
			Memory[StackPointer] = lo_index;
			StackPointer--;
			Memory[StackPointer] = hi_index;
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_A];
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_B];
			StackPointer--;
			Memory[StackPointer] = Flags;
			StackPointer--;
		}
		break;

	default:
		break;
	}
}







void execute(BYTE opcode)
{
	BYTE high_nibble;
	BYTE low_nibble;

	high_nibble = (opcode >> 4) & 0xFF;
	low_nibble = opcode & 0xFF;

	if ((opcode & 0x80) == 0x80)  // Group 1: Dual Operand Instructions
	{
		Group_1_Dual_Operand_Instructions(opcode);
	}
	else if ((opcode & 0xC0) == 0x40)  // Group 2: Single Operand Instructions
	{
		Group_2_Single_Operand_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x00)  // Group 3: TPA, TAP, NOP Instructions
	{
		Group_3_TPA_TAP_NOP_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x08)  // Group 4: Condition Code Instructions
	{
		Group_4_Condition_Code_Instructions(opcode);
	}
	else if ((opcode & 0xF0) == 0x10)  // Group 5: Accumulator Instructions
	{
		Group_5_Accumulator_Instructions(opcode);
	}
	else if ((opcode & 0xF0) == 0x20)  // Group 6: Branch Instructions
	{
		Group_6_Branch_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x30)  // Group 7: Stack and Index Register Instructions
	{
		Group_7_Stack_and_Index_Register_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x38)  // Group 8: Interrupt and Subroutine Instructions
	{
		Group_8_Interrupt_and_Subroutine_Instructions(opcode);
	}
	else
	{
		printf("ERROR> Unrecognised Op-code %X\n", opcode);
	}
}


void emulate_6800()
{
	BYTE opcode;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                  A  B   IX   SP\n");

	while ((!halt) && (memory_in_range))
	{
		printf("%04X ", ProgramCounter);           // Print current address

		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);     // Print Accumulator
		printf("%02X ", Registers[REGISTER_B]);     // Print Register B

		printf("%04X ", IndexRegister);              // Print Stack Pointer

		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_H) == FLAG_H)             // Print Half Carry Flag
		{
			printf("H=1 ");
		}
		else
		{
			printf("H=0 ");
		}

		if ((Flags & FLAG_I) == FLAG_I)             // Print Interrupt Flag
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}

		if ((Flags & FLAG_N) == FLAG_N)             // Print Sign Flag
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}

		if ((Flags & FLAG_Z) == FLAG_Z)             // Print Zero Flag
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}

		if ((Flags & FLAG_V) == FLAG_V)             // Print Overflow Flag
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
		}

		if ((Flags & FLAG_C) == FLAG_C)             // Print Carry Flag
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}




////////////////////////////////////////////////////////////////////////////////
//                      Intel 6800 Simulator/Emulator (End)                   //
////////////////////////////////////////////////////////////////////////////////













void initialise_filenames()
{
	int i;

	for (i = 0; i<MAX_FILENAME_SIZE; i++)
	{
		hex_file[i] = '\0';
		trc_file[i] = '\0';
	}
}




int find_dot_position(char *filename)
{
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		if (chr == '.')
		{
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename)
{
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename)
{
	bool exists;
	FILE *ifp;

	exists = false;

	if ((ifp = fopen(filename, "r")) != NULL)
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename)
{
	FILE *ofp;

	if ((ofp = fopen(filename, "w")) != NULL)
	{
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}





void load_and_run()
{
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	ln = 0;
	chr = '\0';
	while (chr != '\n')
	{
		chr = getchar();

		switch (chr)
		{
		case '\n':
			break;
		default:
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file[ln] = chr;
				trc_file[ln] = chr;
				ln++;
			}
			break;
		}
	}

	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else
	{
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else
	{
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file))
	{
		// Clear Registers and Memory

		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;
		IndexRegister = 0;
		for (i = 0; i<2; i++)
		{
			Registers[i] = 0;
		}
		for (i = 0; i<MEMORY_SIZE; i++)
		{
			Memory[i] = 0xFF;
		}

		// Load hex file

		if ((ifp = fopen(hex_file, "r")) != NULL)
		{
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer))
			{
				if (sscanf(InputBuffer, "L=%x", &address) == 1)
				{
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1)
				{
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
					{
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else
				{
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate the 6800

		emulate_6800();
	}
	else
	{
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}




void test_and_mark()
{
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 0)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1)
			{
				testing_complete = true;
				printf("Current mark = %d\n", mark);
			}
			else if (strcmp(buffer, "Error") == 0)
			{
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else
			{
				// Clear Registers and Memory

				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i = 0; i<2; i++)
				{
					Registers[i] = 0;
				}
				for (i = 0; i<MEMORY_SIZE; i++)
				{
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				while (!end_of_program)
				{
					chr = buffer[i];
					switch (chr)
					{
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1)
						{
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1)
						{
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
							{
								Memory[load_at] = (BYTE)code;
							}
							load_at++;
						}
						else
						{
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				// Emulate the 6800

				if (load_at > 1)
				{
					emulate_6800();

					// Send results

					sprintf(buffer, "%02X%02X %02X%02X %04X %04X %02X%02X %02X%02X", Registers[REGISTER_A], Registers[REGISTER_B], Flags, Memory[TEST_ADDRESS_1], IndexRegister, StackPointer, Memory[TEST_ADDRESS_2], Memory[TEST_ADDRESS_3], Memory[TEST_ADDRESS_4], Memory[TEST_ADDRESS_5]);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Intel 6800 Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1 (2014-15)\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
		// Creation failed!
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock, (SOCKADDR *)&client_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//   //printf("Bind failed! \n");  // Bind failed!
	//}



	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run();
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}