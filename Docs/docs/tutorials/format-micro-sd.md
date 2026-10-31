---
title: "Format a MicroSD Card for PocketMage"
description: "Guide to formatting SD cards for compatibility with PocketMage."
---

# PocketMage SD Card Formatting Guide

::: callout danger "These steps erase the card" Everything on the card is destroyed. Back up your data before you start, and double-check the disk identifier at every step so you do not wipe the wrong device.
:::

This guide formats an SD card so PocketMage can mount it. The device reads a single FAT32 primary partition on an MBR partition table with 32 KB clusters; anything else is not detected reliably.

## Supported SD card requirements

- **Card size:** 2 GB to 32 GB microSD recommended. The slot accepts up to 2 TB, but cards above 32 GB must be forced to FAT32 and MBR and are not recommended.
- **Filesystem:** FAT32
- **Partition scheme:** MBR (Master Boot Record)
- **Partitions:** a single primary partition
- **Cluster (allocation unit) size:** 32 KB
- **Sector size:** 512 bytes (default)

Not supported: exFAT, GPT partition tables, and 64 GB or larger cards unless you explicitly force FAT32 and MBR.

## Windows (recommended)

### Step 1: Open diskpart

Insert the card, open **Command Prompt as Administrator**, and run:

```bat
diskpart
```

### Step 2: Identify the card

```text
list disk
```

Find the disk number that matches your card size.

::: callout warning "Select the correct disk" The next step wipes the disk you select. Confirm the number against the card size before continuing.
:::

### Step 3: Format the card (MBR + FAT32)

Replace `X` with your disk number, then run each line in the `diskpart` prompt:

```text
select disk X
clean
convert mbr
create partition primary
format fs=fat32 unit=32k quick
assign
exit
```

### Step 4: Verify (optional)

Open **Disk Management**, right-click the disk, and open Properties then Volumes. The partition style must read **Master Boot Record (MBR)**.

## macOS (Terminal, not Disk Utility)

Disk Utility often creates GPT layouts that do not work. Use the terminal.

### Step 1: Identify the card

```sh
diskutil list
```

Find the disk number (for example, `disk2`).

### Step 2: Erase and create MBR + FAT32

```sh
diskutil eraseDisk FAT32 PM_SD MBRFormat /dev/disk2
```

### Step 3: Force 32 KB cluster size

```sh
sudo newfs_msdos -F 32 -c 64 /dev/disk2s1
```

`-c 64` sets 64 sectors per cluster; at 512 bytes per sector that is a 32 KB cluster.

### Step 4: Verify

```sh
diskutil info /dev/disk2 | grep "Partition Map"
```

The output must read `Partition Map Scheme: Master Boot Record`.

## Linux

### Step 1: Identify the card

```sh
lsblk
```

Example device: `/dev/sdb`.

### Step 2: Remove existing partition data

```sh
sudo wipefs -a /dev/sdb
```

### Step 3: Create the MBR partition table

```sh
sudo parted /dev/sdb --script mklabel msdos
sudo parted /dev/sdb --script mkpart primary fat32 1MiB 100%
```

### Step 4: Format FAT32 with 32 KB clusters

```sh
sudo mkfs.fat -F 32 -s 64 /dev/sdb1
```

### Step 5: Verify

```sh
lsblk -o NAME,SIZE,PTTYPE
```

`PTTYPE` must read `dos`.

## Final checklist

Before using the card in PocketMage, confirm:

- FAT32 filesystem
- MBR partition table (not GPT)
- one primary partition
- 32 KB cluster size
- card size 32 GB or smaller

If any of these are wrong, the card may not be detected. If problems persist after following this guide, the card itself may be incompatible.
