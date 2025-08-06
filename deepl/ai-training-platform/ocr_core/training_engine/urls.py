from .views import  train_ui, train, get_gpus, get_trains, stop_multiple_trains, label_ui, home
from django.urls import path

urlpatterns = [
    path('label-ui',label_ui, name= "label_ui"),
    path('train-ui', train_ui, name='train_ui'),
    path('train-model', train, name='train-model'),
    path('get-trains', get_trains, name='get_trains'),
    path('stop-trains', stop_multiple_trains, name='stop_multiple_trains'),
    path('get-gpus', get_gpus, name='get_gpus'),
    
    

]  # path('form/<int:id>/', form_detail, name='form_detail')
