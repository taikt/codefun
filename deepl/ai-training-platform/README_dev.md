# 1. install 
```
conda create -n data-label-client python=3.10
conda activate data-label-client
pip install -r requirements.txt
```
# 2. create project & dev
```
django-admin startproject client_form_service
django-admin startapp form_label

```
<!-- django-admin startapp admin_site -->
* đăng ký form_label vào INSTALLED_APPS -> setting.py
form_label

```
python manage.py makemigrations # tạo migration
python manage.py migrate # áp dụng migrate
```

* thêm dữ liệu admin
```
python manage.py shell
from form_label.models import User
super_user = User(username='admin', is_superuser=True)
super_user.set_password('admin123')
```

* thêm app document_alignment
```
django-admin startapp document_alignment
```

* run
```
python manage.py runserver 0.0.0.0:2010
``` 

# 3. Build service
* build docker image
```
sudo docker build -t client_form_service .
```


* install redis
```
sudo docker run -p 6379:6379 redis:latest
```

* db
```
docker run --name mysql-container -e MYSQL_ROOT_PASSWORD=vhoang123 -p 3306:3306 -d mysql:latest


```

* run service
```
sudo docker compose up
```
```
source .env && celery -A main worker --loglevel=info
```

```
mysql -h 192.168.1.104 -P 3306 -u root -p

sudo apt-get install pkg-config python3-dev default-libmysqlclient-dev build-essential
```

```
uvicorn main:app --port 8000
```