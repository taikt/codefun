for ((i=1;i<=500;i++))
do
    sldd dumm startHTTPTransfer --requestParameter="{\"appName\":\"dumm\",\"url\":\"https://uds.e2e.cd-emea.bmw/uds/datatransfer/v1/73af5129-3949-4a68-8170-0d8299ba5f33\",\"httpMethod\":\"GET\",\"httpHeader\":{\"Content-Type\":\"application/octet-stream\"},\"options\":{\"RESPONSE_FILE_PATH\":\"/tmp/test_$i\",\"TRANSFER_FINISHED_TIMEOUT\":\"30000\",\"COMPRESS_BODY_TYPE\":\"None\",\"TRANSFER_STATUS_UPDATE_TIMEOUT\":\"100\"}}" --priority=HIGH
    sleep 2
    top -n 1 > test_$i.txt -p 23464
    
done