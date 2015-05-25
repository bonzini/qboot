#include "bios.h"
#include "ioport.h"
#include "pci.h"
#include "string.h"

// NOTE: this runs from ROM at 0xFFFF0000, so it is not possible to use any
// static data.

#define PIIX_ISA_PIRQA_ROUT 0x60
#define PIIX_PMBASE 0x40
#define PIIX_PMREGMISC 0x80
#define PIIX_SMBHSTBASE 0x90
#define PIIX_SMBHSTCFG 0xd2

static void setup_piix(void)
{
	const int bdf = (1 << 3);
	pci_config_writeb(bdf, PIIX_ISA_PIRQA_ROUT, 10);
	pci_config_writeb(bdf, PIIX_ISA_PIRQA_ROUT+1, 10);
	pci_config_writeb(bdf, PIIX_ISA_PIRQA_ROUT+2, 11);
	pci_config_writeb(bdf, PIIX_ISA_PIRQA_ROUT+3, 11);
}

static void setup_piix_pm(void)
{
	const int bdf = (1 << 3) | 3;

    	pci_config_writel(bdf, PIIX_PMBASE, 0x601);
	pci_config_writeb(bdf, PIIX_PMREGMISC, 0x01);
	pci_config_writel(bdf, PIIX_SMBHSTBASE, 0x701);
	pci_config_writeb(bdf, PIIX_SMBHSTCFG, 0x09);
}

#define ICH9_LPC_PIRQA_ROUT 0x60
#define ICH9_LPC_PIRQE_ROUT 0x68
#define ICH9_LPC_PMBASE 0x40
#define ICH9_LPC_ACPI_CTRL 0x44

static void setup_ich9(void)
{
	const int bdf = 0x1f << 3;
	pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT, 10);
	pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT+1, 10);
	pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT+2, 11);
	pci_config_writeb(bdf, ICH9_LPC_PIRQA_ROUT+3, 11);
	pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT, 10);
	pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT+1, 10);
	pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT+2, 11);
	pci_config_writeb(bdf, ICH9_LPC_PIRQE_ROUT+3, 11);
}

static void setup_ich9_pm(void)
{
	const int bdf = 0x1f << 3;
	pci_config_writel(bdf, ICH9_LPC_PMBASE, 0x601);
	pci_config_writeb(bdf, ICH9_LPC_ACPI_CTRL, 0x80);
}

#define I440FX_PAM0                     0x59
#define Q35_HOST_BRIDGE_PAM0            0x90

static void setup_pic(void)
{
    /* Send ICW1 (select OCW1 + will send ICW4) */
    outb(0x20, 0x11);
    outb(0xa0, 0x11);
    /* Send ICW2 (base irqs: 0x08-0x0f for irq0-7, 0x70-0x77 for irq8-15) */
    outb(0x21, 8);
    outb(0xa1, 0x70);
    /* Send ICW3 (cascaded pic ids) */
    outb(0x21, 0x04);
    outb(0xa1, 0x02);
    /* Send ICW4 (enable 8086 mode) */
    outb(0x21, 0x01);
    outb(0xa1, 0x01);
    /* Mask all irqs (except cascaded PIC2 irq) */
    outb(0x21, ~(1 << 2));
    outb(0xa1, ~0);

    /* Set ELCR to IRQs 10 and 11 */
    outb(0x4d0, 0);
    outb(0x4d1, 0x0c);
}

void setup_hw(void)
{
	const int bdf = 0;
	const uint8_t *bios_start = (uint8_t *)0xffff0000;
	uint8_t *low_start = (uint8_t *)0xf0000;
	int pambase;

        uint32_t id = pci_config_readl(bdf, 0);
        if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_82441 << 16))) {
		setup_piix();
		setup_piix_pm();
		pambase = I440FX_PAM0;
	} else if (id == (PCI_VENDOR_ID_INTEL | (PCI_DEVICE_ID_INTEL_Q35_MCH << 16))) {
		setup_ich9();
		setup_ich9_pm();
		pambase = Q35_HOST_BRIDGE_PAM0;
	} else
		panic();

	// Make ram from 0xc0000-0xf0000 read-write
	int i;
	for (i=0; i<6; i++) {
		int pam = pambase + 1 + i;
		pci_config_writeb(bdf, pam, 0x33);
	}

	// Make ram from 0xf0000-0x100000 read-write and shadow BIOS
	// We're still running from 0xffff0000
	pci_config_writeb(bdf, pambase, 0x30);
	memcpy(low_start, bios_start, 0x10000);

	setup_pic();
}
