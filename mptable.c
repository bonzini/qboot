#include "include/string.h"
#include "bios.h"
#include "fw_cfg.h"
#include "include/mpspec_def.h"

#define MPTABLE_START    0x9fc00
#define APIC_VERSION     0x14
#define MPC_SPEC         0x4

#define MP_IRQDIR_DEFAULT 0
#define MP_IRQDIR_HIGH    1
#define MP_IRQDIR_LOW     3

static const char MPC_OEM[]        = "QBOOT   ";
static const char MPC_PRODUCT_ID[] = "000000000000";
static const char BUS_TYPE_ISA[]   = "ISA   ";

#define IO_APIC_DEFAULT_PHYS_BASE 0xfec00000
#define APIC_DEFAULT_PHYS_BASE    0xfee00000
#define APIC_VERSION              0x14

static int mptable_checksum(char *buf, int size)
{
	int i;
	int sum = 0;

	for (i = 0; i < size; i++) {
		sum += buf[i];
	}

	return sum;
}

static void mptable_get_cpuid(int *signature, int *features)
{
	int ebx, ecx;

	asm("cpuid"
	    : "=a" (*signature), "=b" (ebx), "=c" (ecx), "=d" (*features)
	    : "0" (1));
}

void setup_mptable(void)
{
	struct mpf_intel *mpf;
	struct mpc_table *table;
	struct mpc_cpu *cpu;
	struct mpc_bus *bus;
	struct mpc_ioapic *ioapic;
	struct mpc_intsrc *intsrc;
	struct mpc_lintsrc *lintsrc;
	const char mpc_signature[] = MPC_SIGNATURE;
	const char smp_magic_ident[] = "_MP_";
	int cpuid_stepping, cpuid_features;
	int irq0_override = 0;
	int checksum = 0;
	int offset = 0;
	int num_cpus;
	int ssize;
	int i;

	ssize = sizeof(struct mpf_intel);

	mpf = (struct mpf_intel *) MPTABLE_START;
	memset(mpf, 0, ssize);
	memcpy(mpf->signature, smp_magic_ident, sizeof(smp_magic_ident) - 1);
	mpf->length = 1;
	mpf->specification = 4;
	mpf->physptr = MPTABLE_START + ssize;
	mpf->checksum -= mptable_checksum((char *) mpf, ssize);

	offset += ssize;
	ssize = sizeof(struct mpc_table);

	table = (struct mpc_table *) (MPTABLE_START + offset);
	memset(table, 0, ssize);
	memcpy(table->signature, mpc_signature, sizeof(mpc_signature) - 1);
	table->spec = MPC_SPEC;
	memcpy(table->oem, MPC_OEM, sizeof(MPC_OEM) - 1);
	memcpy(table->productid, MPC_PRODUCT_ID, sizeof(MPC_PRODUCT_ID) - 1);
	table->lapic = APIC_DEFAULT_PHYS_BASE;

	offset += ssize;
	ssize = sizeof(struct mpc_cpu);

	fw_cfg_select(FW_CFG_NB_CPUS);
	num_cpus = fw_cfg_readl_le();
	mptable_get_cpuid(&cpuid_stepping, &cpuid_features);

	for (i = 0; i < num_cpus; i++) {
		cpu = (struct mpc_cpu *) (MPTABLE_START + offset);
		memset(cpu, 0, ssize);
		cpu->type = MP_PROCESSOR;
		cpu->apicid = i;
		cpu->apicver = APIC_VERSION;
		cpu->cpuflag = CPU_ENABLED;
		if (i == 0) {
			cpu->cpuflag |= CPU_BOOTPROCESSOR;
		}
		cpu->cpufeature = cpuid_stepping;
		cpu->featureflag = cpuid_features;
		checksum += mptable_checksum((char *) cpu, ssize);
		offset += ssize;
	}

	ssize = sizeof(struct mpc_bus);

	bus = (struct mpc_bus *) (MPTABLE_START + offset);
	memset(bus, 0, ssize);
	bus->type = MP_BUS;
	bus->busid = 0;
	memcpy(bus->bustype, BUS_TYPE_ISA, sizeof(BUS_TYPE_ISA) - 1);
	checksum += mptable_checksum((char *) bus, ssize);

	offset += ssize;
	ssize = sizeof(struct mpc_ioapic);

	ioapic = (struct mpc_ioapic *) (MPTABLE_START + offset);
	memset(ioapic, 0, ssize);
	ioapic->type = MP_IOAPIC;
	ioapic->apicid = num_cpus + 1;
	ioapic->apicver = APIC_VERSION;
	ioapic->flags = MPC_APIC_USABLE;
	ioapic->apicaddr = IO_APIC_DEFAULT_PHYS_BASE;
	checksum += mptable_checksum((char *) ioapic, ssize);

	offset += ssize;
	ssize = sizeof(struct mpc_intsrc);

	fw_cfg_select(FW_CFG_IRQ0_OVERRIDE);
	irq0_override = fw_cfg_readl_le();

	for (i = 0; i < 16; i++) {
		intsrc = (struct mpc_intsrc *) (MPTABLE_START + offset);
		memset(intsrc, 0, ssize);
		intsrc->type = MP_INTSRC;
		intsrc->irqtype = mp_INT;
		intsrc->irqflag = MP_IRQDIR_DEFAULT;
		intsrc->srcbus = 0;
		intsrc->srcbusirq = i;
		intsrc->dstapic = num_cpus + 1;
		intsrc->dstirq = i;
		if (irq0_override) {
			if (i == 0) {
				intsrc->dstirq = 2;
			} else if (i == 2) {
				// Don't update offset nor checksum
				continue;
			}
		}
		checksum += mptable_checksum((char *) intsrc, ssize);
		offset += ssize;
	}

	ssize = sizeof(struct mpc_lintsrc);

	lintsrc = (struct mpc_lintsrc *) (MPTABLE_START + offset);
	memset(lintsrc, 0, ssize);
	lintsrc->type = MP_LINTSRC;
	lintsrc->irqtype = mp_ExtINT;
	lintsrc->irqflag = MP_IRQDIR_DEFAULT;
	lintsrc->srcbusid = 0;
	lintsrc->srcbusirq = 0;
	lintsrc->destapic = 0;
	lintsrc->destapiclint = 0;
	checksum += mptable_checksum((char *) lintsrc, ssize);

	offset += ssize;

	lintsrc = (struct mpc_lintsrc *) (MPTABLE_START + offset);
	lintsrc->type = MP_LINTSRC;
	lintsrc->irqtype = mp_NMI;
	lintsrc->irqflag = MP_IRQDIR_DEFAULT;
	lintsrc->srcbusid = 0;
	lintsrc->srcbusirq = 0;
	lintsrc->destapic = 0xFF;
	lintsrc->destapiclint = 1;
	checksum += mptable_checksum((char *) lintsrc, ssize);

	offset += ssize;
	ssize = sizeof(struct mpc_table);

	table->length = offset - sizeof(struct mpf_intel);
	checksum += mptable_checksum((char *) table, ssize);
	table->checksum -= checksum;
}
