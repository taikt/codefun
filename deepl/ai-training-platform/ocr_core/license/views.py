from django.shortcuts import render

# Create your views here.
def home(request):
    
    
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

    return render(request, 'home.html', {'info': info})