# Embedded System Course Project (PYNQ-Z1)

This repository is a cleaned-up archive of an Embedded Operating Systems course project targeting **PYNQ-Z1 (Xilinx Zynq-7000 ARM/FPGA SoC)**.

It keeps the core materials needed to reproduce the development workflow:

- Custom PetaLinux recipes (user apps + kernel module)
- Device Tree and U-Boot patch settings
- Standalone driver experiments
- A C programming homework example (HW4)

Large binaries and installers were moved to `artifacts/` and are ignored by `.gitignore` to keep the GitHub repository lightweight.

## 1. Repository Structure

```text
.
|-- README.md
|-- .gitignore
|-- drivers/
|   |-- platformdriver.c
|   |-- char2platform.c
|   `-- intr_module/intr_driver.c
|-- petalinux/
|   |-- system-user.dtsi
|   |-- recipes-apps/
|   |   |-- intrapp/
|   |   |-- gpio-demo/
|   |   |-- peekpoke/
|   |   `-- userapp/
|   |-- recipes-modules/
|   |   `-- intrdriver/
|   `-- meta-user/recipes-bsp/
|       |-- device-tree/files/system-user.dtsi
|       `-- u-boot/
|-- hardware/
|   `-- system_wrapper.xsa
|-- homework/
|   `-- HW4/
`-- artifacts/   (large files kept locally, not recommended for version control)
```

## 2. Module Overview

### 2.1 `petalinux/recipes-modules/intrdriver`

- Custom kernel module recipe: `intrdriver.bb`
- Kernel source: `files/intrdriver.c`
- Compatible string: `compatible = "xlnx,intr"`
- Registers an IRQ and reads IP register data on interrupt trigger (offset `0x4`)

### 2.2 `petalinux/recipes-apps`

- `intrapp`: Interactive `/dev/mem` control app for GCD / DES / AES / INTR IPs
- `gpio-demo`: Xilinx GPIO sysfs demo application
- `peekpoke`: Memory read/write utilities (`peek` / `poke`)
- `userapp`: Basic Hello World example

### 2.3 `petalinux/system-user.dtsi`

- Defines PL-side IP nodes (`gcd`, `des`, `aes`, `intr`)
- Includes interrupt settings (`interrupt-parent`, `interrupts`)
- Synced to `petalinux/meta-user/recipes-bsp/device-tree/files/system-user.dtsi`

### 2.4 `drivers/`

- `platformdriver.c`: Basic platform driver + IRQ version
- `char2platform.c`: Platform driver with character device interface (`/dev/myhwip`)
- `intr_module/intr_driver.c`: Standalone interrupt-module version for class experiments

### 2.5 `homework/HW4`

- Multi-file C homework (`main.c`, `input.c`, `calculate.c`)
- Features: score input, range statistics, max/min/average

## 3. Recommended Development Environment

- OS: Ubuntu 20.04 (or course-required Linux version)
- PetaLinux: **2020.2**
- Board: PYNQ-Z1
- Toolchain: Xilinx tools (Vivado / Vitis, based on your class flow)

## 4. Quick Start (PetaLinux Integration Flow)

> Commands below assume a Linux host environment. Adjust paths for your setup.

1. Create a new project and import hardware description

```bash
petalinux-create -t project --template zynq --name esys-pynqz1
cd esys-pynqz1
petalinux-config --get-hw-description=/path/to/repo/hardware
```

2. Copy recipes into `project-spec/meta-user`

```bash
cp -r /path/to/repo/petalinux/recipes-apps project-spec/meta-user/
cp -r /path/to/repo/petalinux/recipes-modules project-spec/meta-user/
cp -r /path/to/repo/petalinux/meta-user/recipes-bsp/* project-spec/meta-user/recipes-bsp/
cp /path/to/repo/petalinux/system-user.dtsi project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi
```

3. Enable apps/modules in rootfs config

```bash
petalinux-config -c rootfs
```

Then enable:

- `user packages -> intrapp`
- `user packages -> gpio-demo`
- `user packages -> peekpoke`
- `user packages -> userapp`
- `kernel modules -> intrdriver`

4. Build images

```bash
petalinux-build
```

5. Package boot files (adjust to your class workflow)

```bash
petalinux-package --boot --u-boot
```

## 5. Common Runtime Tests

### 5.1 Load module and inspect logs

```bash
insmod intrdriver.ko
dmesg | tail -n 50
```

### 5.2 Test user applications

```bash
intrapp
gpio-demo -g 240 -o 1
peek 0x43C30004
poke 0x43C30000 0x1
```

## 6. GitHub Upload Checklist

1. Initialize repository and inspect staged files:

```bash
git init
git add .
git status
```

2. Confirm large files in `artifacts/` are not staged.
3. Suggested first commit message:

```bash
git commit -m "chore: reorganize PYNQ-Z1 embedded systems course project"
```

## 7. Cleanup Work Already Done

- Reorganized structure into `petalinux/`, `drivers/`, `homework/`, `hardware/`, `artifacts/`
- Removed noise files (`*.o`, HW4 binary, swap files)
- Synced final `system-user.dtsi` into the `meta-user/recipes-bsp` location

## 8. Notes

- This repository is a course-code/configuration collection, not a fully self-contained PetaLinux project folder.
- Some apps use direct MMIO access via `/dev/mem`; production systems should use controlled driver interfaces.
- `intrapp.c` contains legacy encoded text in prompts/comments from historical source files; this does not affect repository organization.

## 9. TODO (Optional)

- Add a complete `project-spec/meta-user/conf/layer.conf` (if you want this to behave like a standalone layer)
- Add a script/Makefile to automate recipe copy into a PetaLinux project
- Add minimal validation scripts for driver and app testing
