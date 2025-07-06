# Installing Miniconda on Windows

## 1. Download Miniconda

Go to the official Miniconda download page:

➡️ [https://docs.conda.io/en/latest/miniconda.html](https://docs.conda.io/en/latest/miniconda.html)

Download the Windows installer:

* Choose the **64-bit installer** for **Python 3.x**, e.g., *Miniconda3 Windows 64-bit*.

---

## 2. Run the Installer

Follow the installation steps:

* Accept the license agreement
* Choose **"Just Me"** (unless you want to install for all users)
* (Optional) Check **"Add Miniconda to my PATH environment variable"** for convenience
* Allow it to **initialize Conda** (recommended)

---

## 3. Verify Installation

Open **Command Prompt** and run:

```bash
conda --version
```

You should see something like:

```bash
conda 24.3.0
```

---

## 4. Set Up Your Environment

To create and activate the environment:

```bash
conda env create -f environment.yml
conda activate led-ui
python main.py
```

---

## 5. Update Dependencies (Optional)

If you change or add dependencies in `environment.yml`, update the environment:

```bash
conda env update --file environment.yml --prune
```

The `--prune` flag removes any dependencies no longer required.
