from django.shortcuts import render
from django.core.files.storage import FileSystemStorage
from django.http import JsonResponse
from .models import InferModel
import requests
from urllib.parse import urlparse

from django.views.decorators.csrf import csrf_exempt
from .utils import convert_pdf_bytes_to_image_arrays, convert_image_bytes_to_array, download_file
from .AI_services import ocr_line, ocr_by_form
from .config import FORM_PATH
import pathlib
import os
import json
# Create your views here.
PDF_EXTENSION = ['.pdf','.PDF']
IMG_EXTENSION = ['.jpg','.JPG','.png','.PNG','.jpeg','.JPEG']

@csrf_exempt
def update_model(request):
    if request.method == 'POST':
        data = json.loads(request.body)
        code = data.get('code')
        inference_path = data.get('inference_path')
        inference = InferModel.objects.filter(code=code)
        local_path = download_file(inference_path, FORM_PATH)
        if local_path is not None:
            if inference.exists():
                obj = inference[0]
                # old_local_path = obj.inference_path 
                # os.remove(old_local_path)
                obj.inference_path = local_path
                obj.save()
            else:
                inference = InferModel.objects.create(code=code,
                                                    inference_path=local_path)
            return JsonResponse({"status":1,"message":"Success"})
    return JsonResponse({"status":0,"message":"Failed"})

@csrf_exempt
def ocr_free(request):
    if request.method == 'POST' and request.FILES['ocr_file']:
        myfile = request.FILES['ocr_file']
        filebytes = myfile.read()
        # print(type(filebytes))
        extension = pathlib.Path(myfile.name).suffix
        if extension in PDF_EXTENSION:
            np_array = convert_pdf_bytes_to_image_arrays(filebytes)
        elif extension in IMG_EXTENSION:
            np_array = convert_image_bytes_to_array(filebytes)
            np_array = np_array[:, :, :3]
            # print(np_array.shape)
        else:
            return JsonResponse({"status":0,"message":"Wrong extension"})
        result = ocr_line(np_array)

        return JsonResponse({"status":1,"message":"Success","data":result})
    return JsonResponse({"status":0,"message":"Wrong method"})

@csrf_exempt
def ocr_form(request):
    if request.method == 'POST' and request.FILES['ocr_file']:
        myfile = request.FILES['ocr_file']
        code = request.POST.get('code')
        filebytes = myfile.read()
        # print(type(filebytes))
        extension = pathlib.Path(myfile.name).suffix
        if extension in PDF_EXTENSION:
            np_array = convert_pdf_bytes_to_image_arrays(filebytes)
        elif extension in IMG_EXTENSION:
            np_array = convert_image_bytes_to_array(filebytes)
            np_array = np_array[:, :, :3]
        else:
            return JsonResponse({"status":0,"message":"Wrong extension"})
        inference = InferModel.objects.filter(code=code)
        if inference.exists():
            inference_path = inference[0].inference_path
            result = ocr_by_form(np_array,inference_path)
            return JsonResponse({"status":1,"message":"Success","data":result})
    return JsonResponse({"status":0,"message":"Wrong method"})
        
@csrf_exempt
def ocr_by_url(request):
    if request.method == 'POST':
        data = json.loads(request.body)
        code = data.get('code')
        url = data.get('url')
        parsed_url = urlparse(url)
        file_name = os.path.basename(parsed_url.path)
        try:
            response = requests.get(url, stream=True)
            if response.status_code == 200:
                filebytes = response.content
                extension = pathlib.Path(file_name).suffix
                if extension in PDF_EXTENSION:
                    np_array = convert_pdf_bytes_to_image_arrays(filebytes)
                elif extension in IMG_EXTENSION:
                    np_array = convert_image_bytes_to_array(filebytes)
                    np_array = np_array[:, :, :3]
                else:
                    return JsonResponse({"status":0,"message":"Wrong extension"})
                inference = InferModel.objects.filter(code=code)
                if inference.exists():
                    inference_path = inference[0].inference_path
                    result = ocr_by_form(np_array,inference_path)
                    return JsonResponse({"status":1,"message":"Success","data":result})
            else:
                return JsonResponse({"status":0,"message":"Can not get image url"})
        
        except Exception as e:
            print(e)
            return JsonResponse({"status":0,"message":"Can not get image url"})
    return JsonResponse({"status":0,"message":"Wrong method"})
        
        