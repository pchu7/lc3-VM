#include <stdio.h>


/* 65536 total memory locations */
uint16_t memory[UINT16_MAX];

enum
{
	R_R0 = 0,
	R_R1,
	R_R2,
	R_R3,
	R_R4,
	R_R5,
	R_R6,
	R_R7,
	R_PC,		/* Program Counter */
	R_COND,
	R_COUNT
};

uint16_t reg[R_COUNT];

/* Enum defining the 16 different possible assembly operations */
enum
{
	OP_BR = 0; 	/* Branch */
	OP_ADD,		/* Add */
	OP_LD,		/* Load */
	OP_ST,		/* Store */
	OP_JSR,		/* Jump register */
	OP_AND,		/* Bitwise AND */	
	OP_LDR,		/* Load Register */
	OP_STR, 	/* Store Register */
	OP_RTI,		/* Unused */
	OP_NOT,		/* Bitwise Not */
	OP_LDI,		/* Load Indirect */
	OP_STI, 	/* Store Indirect */
	OP_JMP,		/* Jump */
	OP_RES,		/* Reserved (Unused) */
	OP_LEA,		/* Load Effective Address */
	OP_TRAP		/* Execute Trap */
}

/* Enum defining the 3 different types of condition flags */
enum
{
	FL_POS = ( 1 << 0 ); /* Positive Flag */
	FL_ZRO = ( 1 << 1 ); /* Zero Flag */
	FL_NEG = ( 1 << 2 ); /* Negative Flag */
}

/* Sign extends the provided value to maintain the sign of the number up to 16 bits */
uint16_t sign_extend( uint16_t x, int bit_count ) 
{
	if ( ( x >> ( bit_count - 1 ) ) & 1 ) 
	{
		x |= ( 0xffff << bit_count );
		return x;
	}
}

/* Updates the flags based on the result of the calculation */
void update_flags( uint16_t r )
{
	if ( reg[r] == 0 )
	{
		reg[R_COND] = FL_ZRO;
	}
	/* If the leftmost bit is 1, then the nunmber is negative */
	else if ( reg[r] >> 15 ) {
		reg[R_COND] - FL_NEG;
	}
	else
	{
		reg[R_COND] = FL_POS;
	}
}

int main(int argc, const char* argv()) 
{
	if ( argc < 2)
	{
		/* Display usage string */
		printf("lc3 [image-file1] ...\n");
		exit( 2 );
	}

	for ( int j = 1; j < argc; j++ )
	{
		if ( !read_image(argv[j]))
		{
			printf( "failed to load image: %s\n", argv[j] );
			exit( 1 );
	
	}
	
	/* Zero flag is set initially since only one flag should be set at any given tim*/
	reg[R_COND] = FL_ZRO;
	
	enum 
	{
		PC_START = 0x3000;
	}
	reg[R_PC] = PC_START;

	int running = 1;
	while ( running )
	{
		

		/* INSTRUCTION FETCH */
		uint16_t instr = mem_read( reg[R_PC]++ );
		uint16_t op = ( instr >> 12 );
		switch ( op )
		{
				case OP_ADD:
					/* Grab the destination registor */
					uint16_t r0 = ( instr >> 9 ) & 0x7;
					/* Grab the first operand */
					uint16_t r1 = ( instr >> 6 ) & 0x7;
					/* Set flag to determine whether or not we are in immediate mode */
					uint16_t imm_flag = ( instr >> 5 ) & 0x1;
					/* If the immediate bit is set */
					if ( imm_flag )
					{
						/* Sign extend the immediate value */
						uint16_t imm5 = sign_extend(instr & 0x1f, 5);
						/* Add the two values */
						reg[r0] = reg[r1] + imm5;
					}
					else {
						/* Add value loaded from register */
						uint16_t r2 = instr & 0x7;
						reg[r0] = reg[r1] + reg[r2];
					}
					update_flags( r0 );

				case OP_AND:
					/* Grab dest register */
					uint16_t r0 = ( instr >> 9 ) & 0x7;
					/* Grab first operand */
					uint16_t r1 = ( instr >> 6 ) & 0x7;

					uint16_t imm_flag = ( instr >> 5 ) & 0x1;

					if ( imm_flag )
					{

						/* Sign extend the immediate value */
						uint16_t imm5 = sign_extend(instr & 0x1f, 5);
						reg[r0] = reg[r1] & imm5;
					}
					else
					{
						uint16_t r2 = instr & 0x7;
						reg[r0] = reg[r1] & reg[r2];
					}
					update_flags( r0 );
					
				case OP_NOT:
					/* Grab dest register */
					uint16_t r0 = ( instr >> 9 ) & 0x7;
					/* Grab first operand */
					uint16_t r1 = ( instr >> 6 ) & 0x7;

					reg[r0] = -reg[r1];
					update_flags( r0 );

				case OP_BR:
					/* Grab pc offset */
					uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);
					uint16_t cond_flag = ( instr >> 9 ) & 0x7;
					if ( cond_flag & reg[R_COND] )
					{
						reg[R_PC] += pc_offset;
					}
				case OP_JMP:
					uint16_t r1 = ( instr >> 6 ) & 0x7;
					reg[R_PC] = reg[r1];
				case OP_JSR:
				case OP_LD:
					uint16_t r0 = ( instr >> 9 ) & 0x7;
					uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);
					reg[r0] = mem_read( reg[R_PC] + pc_offset );
					update_flags( r0 );
				case OP_LDI:
					/* Grab the destination register */
					uint16_t r0 = ( instr >> 9 ) & 0x7;
					/* Grab PCOffset9 and sign extend */
					uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);
					/* Add pc_offset to current PC and read from resulting address */
					reg[r0] = mem_read( mem_read(reg[R_PC] + pc_offset ) );
					update_flags( r0 );

				case OP_LDR:
				case OP_LEA:
				case OP_ST:
				case OP_STI:
				case OP_STR:
				case OP_TRAP:
				case OP_RES:
				case OP_RTI:
				default:

					break;
		}

}
