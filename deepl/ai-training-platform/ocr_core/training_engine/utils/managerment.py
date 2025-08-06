# def update_status(id, status):
#     try:
#         payload = {"session_name":id, "status": status}
#         headers = {'Content-Type': 'application/json'}
#         # print(payload)
#         response = requests.request("POST", config.STATUS_CALLBACK, headers=headers, data=json.dumps(payload))
#         # print(response.status_code)
#         if response.status_code==200:
#             return True
#         return False
#     except Exception as e:
#         print(e)
#         return False

# def update_progress(id, epoch, epochs):
#     try:
        
#         payload = {"session_name":id, "epoch": epoch, "epochs":epochs}
#         headers = {'Content-Type': 'application/json'}
#         response = requests.request("POST", config.PROGRESS_CALLBACK, headers=headers, data=json.dumps(payload))
#         if response.status_code==200:
#             return True
#         return False
#     except Exception as e:
#         print(e)
#         return False
    
# def update_accuracy(id, accuracy):
#     try:
#         payload = {"session_name":id, "accuracy": accuracy}
#         headers = {'Content-Type': 'application/json'}
#         response = requests.request("POST", config.ACCURACY_CALLBACK, headers=headers, data=json.dumps(payload))
#         if response.status_code==200:
#             return True
#         return False
#     except Exception as e:
#         print(e)
#         return False
import os
import django
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'ocr_core.settings')
django.setup()
from ..models import TrainSession

def update_status(id, status):
    # try:
        session = TrainSession.objects.filter(session_name=id).first()
        session.status = status
        session.save()

        return True
    # except Exception as e:
    #     print(e)
    #     return False
    
def update_progress(id, epoch, epochs):
    # try:
        session = TrainSession.objects.filter(session_name=id).first()
        session.progress = 100*int(epoch)/int(epochs)
        session.save()
        return True
    # except Exception as e:
    #     print(e)
    #     return False

def update_path(id, model_path, inference_path):
    # try:
        session = TrainSession.objects.filter(session_name=id).first()
        session.model_path = model_path
        # session.config_path = config_path
        session.inference_path = inference_path
        session.save()
        return False
    # except Exception as e:
    #     print(e)
    #     return False

def update_accuracy(id, accuracy):
    # try:
        session = TrainSession.objects.filter(session_name=id).first()
        session.accuracy = accuracy
        session.save()

        return True
    # except Exception as e:
    #     print(e)
    #     return False