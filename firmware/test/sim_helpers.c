/***************************************************************************
 *   Copyright (C) 2016 by Thibault Bouttevin                              *
 *   thibault.bouttevin@gmail.com                                          *
 *   www.legalethurlant.fr.st                                              *
 *                                                                         *
 *   This file is part of SolexTronic                                      *
 *                                                                         *
 *   SolexTronic is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 /**
 * \file sim_helpers.c
 * \brief Helpers function to increase usability of simavr 
 * \author Thibault Bouttevin
 * \date Feb 2016
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
#include "sim_helpers.h"


#define V(msg, ...) do{fprintf(stdout, "Helper: "); fprintf(stdout, msg, ##__VA_ARGS__ );}while(0)
//#define V(msg, ...) do{}while(0)

#define MAX_16BIT_IRQ 16
typedef struct {
    avr_irq_t *irq;
    uint16_t value;
    int num;
    char name[32];
}irq_16_t;
static irq_16_t irq_16bit_pool[MAX_16BIT_IRQ];
static int irq_16bit_count = 0;

#define MAX_SYMBOL_IRQ 16
typedef struct {
    avr_irq_t *irq;
    int num;
    uint32_t addr;
    char name[32];
    int size;
}irq_symbol_t;
static irq_symbol_t irq_symbol_pool[MAX_SYMBOL_IRQ];
static int irq_symbol_count = 0;

#define MAX_DEBUG_IRQ 16
typedef struct {
    avr_irq_t *irq;
    int num;
    uint32_t addr;
    char name[32];
    int size;
}irq_debug_t;
static irq_debug_t irq_debug_pool[MAX_DEBUG_IRQ];
static int irq_debug_count = 0;

/*
 * Called on high byte change
 */
static void helpers_high_byte_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
    irq_16_t *irq16 = (irq_16_t*) param;
    irq16->value = (irq16->value & 0xFF) + ((value & 0xFF) << 8);
    //V("High byte changed for IRQ %d : new value 0x%x\n", irq16->num, irq16->value);
    avr_raise_irq(irq16->irq, irq16->value);
}

/*
 * Called on low byte change
 */
static void helpers_low_byte_hook(struct avr_irq_t * irq, uint32_t value, void * param)
{
    irq_16_t *irq16 = (irq_16_t*) param;
    irq16->value = (irq16->value & 0xFF00) + (value & 0xFF);
    //V("Low byte changed for IRQ %d : new value 0x%x\n", irq16->num, irq16->value);
    avr_raise_irq(irq16->irq, irq16->value);
}

/*
 * Called on debug reg write : simply raise corresponding irq
 */
static void helpers_debug_write_hook(struct avr_t *avr, avr_io_addr_t addr, uint8_t value, void * param)
{
    irq_debug_t *irqdebug = (irq_debug_t*) param;
    //V("Debug write for IRQ %d : new value 0x%x\n", irqdebug->num, value);
    avr_raise_irq(irqdebug->irq, value);
}

/**
 * Create 16 bits IRQ to easily track 16 bits registers like timers or variable
 **/
avr_irq_t*
helpers_register_16bit_irq(
		struct avr_t * avr,
		uint32_t base_address,
		const char *name)
{
    if(irq_16bit_count == (MAX_16BIT_IRQ - 1))
    {
        printf("No more space in 16 bits IRQ pool !\n");
        return NULL;
    }
    irq_16bit_count++;
	char *pName = &(irq_16bit_pool[irq_16bit_count].name[0]);
	strncpy(pName, name, 32);
    const char *ppName[] = {pName};
    irq_16bit_pool[irq_16bit_count].num = irq_16bit_count;
	irq_16bit_pool[irq_16bit_count].irq = avr_alloc_irq(&avr->irq_pool, 0, 1, ppName);
    avr_irq_register_notify(avr_iomem_getirq(avr, base_address, "low", 8), 
            helpers_low_byte_hook, &irq_16bit_pool[irq_16bit_count]);
    avr_irq_register_notify(avr_iomem_getirq(avr, base_address+1, "high", 8), 
            helpers_high_byte_hook, &irq_16bit_pool[irq_16bit_count]);
    
    V("Register 16 bits IRQ %s on regs 0x%X:%X\n", name, base_address, base_address+1);
    
    return irq_16bit_pool[irq_16bit_count].irq;
}

/**
 * Create IRQ on symbol to easily track static variable.
 * Name might be a struct member with "struct.member" syntax
 * Only u/s8 and u/s16 are supported (so supported 'size' are 8 or 16  
 **/
avr_irq_t*
helpers_register_symbol_irq(
		struct avr_t * avr,
        elf_firmware_t *f,
		const char *name,
        int size)
{
    if(irq_symbol_count == (MAX_SYMBOL_IRQ - 1))
    {
        printf("No more space in symbol IRQ pool !\n");
        return NULL;
    }
    if((size != 16) && (size != 8))
    {
        printf("Unsupported size %d (only 8 or 16)\n", size);
        return NULL;
    }

    irq_symbol_count++;
	char *pName = &(irq_symbol_pool[irq_symbol_count].name[0]);
	strncpy(pName, name, 32);
    const char *ppName[] = {pName};
    irq_symbol_pool[irq_symbol_count].num = irq_symbol_count;
    irq_symbol_pool[irq_symbol_count].size = size;
    /* look for symbol address and size
     * for 8 bits, just connect IRQ
     * for 16bits, use a 16bit IRQ
     */
    int i;
    for(i=0; i < f->symbolcount; i++)
    {
        if(!strncmp(f->symbol[i]->symbol, name, 32))
        {
            V("Symbol %i : addr 0x%X : %s\n", i, f->symbol[i]->addr, f->symbol[i]->symbol);
            irq_symbol_pool[irq_symbol_count].addr = f->symbol[i]->addr & 0xFFFF; // shift of 0x800000 in symbol       
            break;
        }
    } 
    if(i == f->symbolcount)
    {
        printf("Symbol %s not found! \n", name);
        return NULL;
    }
    if(size == 8)
    {
        V("Symbol 8 bit %s : addr 0x%X\n", name, irq_symbol_pool[irq_symbol_count].addr);
        irq_symbol_pool[irq_symbol_count].irq = avr_alloc_irq(&avr->irq_pool, 0, 1, ppName);
        V("alloc done\n");
        // TODO doesn't work of SRAM, only IO (lower 280 bytes)
        avr_irq_t *ioirq =avr_iomem_getirq(avr, irq_symbol_pool[irq_symbol_count].addr, name, 8); 
        V("get_irq done\n");
        avr_connect_irq(ioirq,
                        irq_symbol_pool[irq_symbol_count].irq);
        V("connect done\n");
    }
    else if(size == 16)
    {
        irq_symbol_pool[irq_symbol_count].irq = 
            helpers_register_16bit_irq(avr, irq_symbol_pool[irq_symbol_count].addr, name);
    }
    
    V("Symbol IRQ %s on regs 0x%X\n", name, irq_symbol_pool[irq_symbol_count].addr);
    
    return irq_symbol_pool[irq_symbol_count].irq;
}

/**
 * Create IRQ on unused IO register to get several debug values 
 * Only u/s8 and u/s16 are supported (so supported 'size' are 8 or 16
 * Supported address range is 0xC7 to 0xFF for Atmega 328  
 **/
avr_irq_t*
helpers_register_debug_irq(
		struct avr_t * avr,
		uint32_t base_address,
		const char *name,
        int size)
{
    if(irq_debug_count == (MAX_DEBUG_IRQ - 1))
    {
        printf("No more space in debug IRQ pool !\n");
        return NULL;
    }
    if((size != 16) && (size != 8))
    {
        printf("Unsupported size %d (only 8 or 16)\n", size);
        return NULL;
    }

    irq_debug_count++;
	char *pName = &(irq_debug_pool[irq_debug_count].name[0]);
	strncpy(pName, name, 32);
    const char *ppName[] = {pName};
    irq_debug_pool[irq_debug_count].num = irq_debug_count;
    irq_debug_pool[irq_debug_count].size = size;
    /* for 8 bits, just connect IRQ
     * for 16bits, use a 16bit IRQ
     */
    if(size == 8)
    {
        irq_debug_pool[irq_debug_count].irq = avr_alloc_irq(&avr->irq_pool, 0, 1, ppName);
        avr_irq_t *ioirq =avr_iomem_getirq(avr, base_address, name, 8); 
        avr_connect_irq(ioirq,
                        irq_debug_pool[irq_debug_count].irq);
        // activate watch on write 
        avr_register_io_write(avr, base_address, helpers_debug_write_hook, &irq_debug_pool[irq_debug_count]);
        V("debug IRQ %s on regs 0x%x\n", name, base_address);
    }
    else if(size == 16)
    {
        irq_debug_pool[irq_debug_count].irq = 
            helpers_register_16bit_irq(avr, base_address, name);
        // activate watch on write 
        avr_register_io_write(avr, base_address, helpers_debug_write_hook, &irq_debug_pool[irq_debug_count]);
        V("debug IRQ %s on regs 0x%X:%X\n", name, base_address, base_address+1);
    }
    
    
    return irq_debug_pool[irq_debug_count].irq;
}

