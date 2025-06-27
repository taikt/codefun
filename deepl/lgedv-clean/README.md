# check port
sudo netstat -tulpn | grep :8080

# start server
python3 optimized_viewer.py

# build
npm run compile
vsce package