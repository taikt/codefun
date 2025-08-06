from .views import  update_model, ocr_free, ocr_form, ocr_by_url
from django.urls import path

urlpatterns = [
    path('updateModel',update_model, name= "update_model"),
    path('ocrLines', ocr_free, name='ocr_free'),
    path('ocrForm', ocr_form, name='ocr_form'),
    path('ocrFormURL',ocr_by_url, name='ocr_by_url')

]  # path('form/<int:id>/', form_detail, name='form_detail')
