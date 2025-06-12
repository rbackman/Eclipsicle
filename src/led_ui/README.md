conda env create -f environment.yml
conda activate led-ui
python main.py

// update dependencies
conda env update --file environment.yml --prune