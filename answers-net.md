a good way to know what happend is to grep `#ifdef LAB_NET`

e1000 will attach to pcie... so that's a lots of init steps...

```sh
QEMUOPTS += -device e1000,netdev=net0,bus=pcie.0

```
new mmio
```c
#ifdef LAB_NET
  // PCI-E ECAM (configuration space), for pci.c
  kvmmap(kpgtbl, 0x30000000L, 0x30000000L, 0x10000000, PTE_R | PTE_W);
  // pci.c maps the e1000's registers here.
  kvmmap(kpgtbl, 0x40000000L, 0x40000000L, 0x20000, PTE_R | PTE_W);
#endif
```

`pci_init`
- first scan dev on bus 0, e1000 has `id = 100e8086`... that's interesting
- then, pcie place e1000 at `0x40000000L`?
- anyway, we can use e1000 by normal mmio


calling stack
```
sockwrite -> net_tx_udp -> net_tx_ip -> net_tx_eth -> e1000_transmit
usertrap -> e1000_intr -> e1000_recv -> net_rx -> ip or arp...
```
- `e1000_transmit` need lock, since it will be called from proc
- `e1000_recv` is called by external intr

> rtfh (reading the fucking hint...)

Transmit Descriptor Tail register (TDT)
> This register holds a value which is an offset from the base, and indicates the location beyond
  the last descriptor hardware can process. This is the location where software writes the first
  new descriptor.

Transmit Descriptor Command Field Format
> EOP: When set, indicates the last descriptor making up the packet. One or many
  descriptors can be used to form a packet

> RS: When set, the Ethernet controller needs to report the status information. This ability
  may be used by software that does in-memory checks of the transmit descriptors to
  determine which ones are done and packets have been buffered in the transmit
