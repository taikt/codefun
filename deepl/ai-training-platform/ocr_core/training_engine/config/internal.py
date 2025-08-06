import os 
from dotenv import load_dotenv
load_dotenv()

# BACKEND_SERVER = os.environ.get('BACKEND_SERVER')
# PROGRESS_CALLBACK = BACKEND_SERVER+"/update-progress/"
# STATUS_CALLBACK = BACKEND_SERVER+"/update-status/"
# ACCURACY_CALLBACK = BACKEND_SERVER+"/update-accuracy/"
TRAIN_LIMIT = 2 if not os.environ.get('TRAIN_LIMIT') else int(os.environ.get('TRAIN_LIMIT'))

TRAIN_DATASET = "training_engine/datasets"
TRAIN_LOG = "training_engine/logs"
TRAIN_EXPERIMENT ="training_engine/experiments"

# TRAIN_TASK = 'train'

