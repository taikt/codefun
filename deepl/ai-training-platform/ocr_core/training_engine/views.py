from django.shortcuts import render
from rest_framework.decorators import api_view
from django.http import JsonResponse
from rest_framework import status
from datetime import datetime
from django.conf import settings
import platform
import os
import signal

from .models import TrainSession
from .core import train_model
from .config import TRAIN_LIMIT
from django.utils import timezone

from .utils import get_gpu_info, generate_unique_id
import json
from django.views.decorators.csrf import csrf_exempt
from multiprocessing import Process
from django.http import HttpResponse
from django.middleware.csrf import get_token
import torch
import torch.multiprocessing as mp
mp.set_start_method('spawn', force=True)

training_session = []

def label_ui(request):
    code = request.GET.get('code', None) 
    return render(request, 'label.html', {'code': code})

def train_ui(request):
    code = request.GET.get('code', None) 
    return render(request, 'train.html', {'code': code})

# Create your views here.
@csrf_exempt
def train(request):
    if request.method == 'POST':
        data = json.loads(request.body)
        form_code = data.get('code')
        trains = TrainSession.objects.filter(status="TRAINING")
        if len(trains) > TRAIN_LIMIT:
            return JsonResponse({'success': False, 'error': 'num train limited'}, status=200)
        train_session_id = generate_unique_id()
        process = Process(target=train_model, args=(form_code, train_session_id))
        
        process.start()
        train_session = TrainSession.objects.create(session_name = str(train_session_id), 
                                                        code= form_code,
                                                        start_time = timezone.now(),
                                                        end_time = timezone.now(),
                                                        status = "INITIATED",
                                                        pid = process.pid)

        train_session.save()
        training_session.append((train_session_id,process,process.pid))
        return JsonResponse({'success': True, 'message': 'Done'}, status=200)
        
    return JsonResponse({'success': False, 'error': 'Invalid method'}, status=405)

def get_trains(request):
    try:
        code = request.GET.get('code')
        trains = TrainSession.objects.filter(code=code).values("id","session_name","status","progress","accuracy")
        
        return JsonResponse({'trains': list(trains)}, status=200)
    except:
        return JsonResponse({'status': 'error', 'message': 'code is required'}, status=400)

    
def stop_multiple_trains(request):
    try:
        data = json.loads(request.body)
        train_ids = data.get('train_ids')#data['train_ids']#
        if not train_ids:
            return JsonResponse({'success': False, "error": "No form IDs provided."}, status=status.HTTP_400_BAD_REQUEST)
        trains = TrainSession.objects.filter(id__in=train_ids, status="TRAINING")
        # print(trains)
        if not trains.exists():
            return JsonResponse({'success': False, "error": "No forms found with the provided IDs."}, status=status.HTTP_404_NOT_FOUND)
        for tn in trains:
            os.kill(tn.pid, signal.SIGTERM)
            tn.status = "TERMINATED"
            tn.save()
            
        return JsonResponse({'success': True, "message": "Trains stopped successfully!"}, status=200)
    except:
        return JsonResponse({'success': False, "error": "Core failed"}, status=status.HTTP_400_BAD_REQUEST)



def get_gpus(request):
    gpu_info = get_gpu_info()
    if gpu_info is not None:

        return JsonResponse({"gpu_info": gpu_info})
    else:
        return JsonResponse({"gpu_info":[]})
    
def home(request):
    # Lấy thông tin về hệ điều hành
    os_info = platform.uname()
    gpu_info = get_gpu_info()
    # Tạo dictionary chứa thông tin cấu hình
    info = {
        "producer":{
            "company": "CÔNG TY TNHH PHẦN MỀM SAVIS",
            "dkkd": "01100459445"
        },
        "customer":
        {
            "name": "SỞ THÔNG TIN VÀ TRUYỀN THÔNG TỈNH BẮC GIANG",
            "address": "Đường Hoàng Văn Thụ, Phường Xương Giang, Thành phố Bắc Giang, Tỉnh Bắc Giang"
        },
        "detail":
        {
            "title": "Phần mềm SAVIS OCR",
            "code": "SAVIS OCR",
            "type": "Vĩnh viễn",
            "num": 1,
            "exp": "12 tháng kể từ ngày nghiệm thu",
            "update_time": "05 năm",
            "license": "AQXCER-14JLYI-2NMDF5-6BCSEA",
            "active_day": "26/12/2024"
        }
        
    }

    return render(request, 'home.html', {'info': info}) #JsonResponse(server_config)

def get_csrf_token(request):
    csrf_token = get_token(request)
    return JsonResponse({'csrfToken': csrf_token})



