#!/bin/sh
mkdir -p ~/.kube
echo "$DEV_KUBECONFIG" > ~/.kube/config
export KUBECONFIG=~/.kube/config
export CURRENT_TIMESTAMP=$(date +%s)
echo $CI_COMMIT_TAG
echo $CI_PROJECT_NAME
echo "Current Timestamp: $CURRENT_TIMESTAMP"

kubectl config set-context --current --namespace=$CI_PROJECT_ROOT_NAMESPACE

#build secret deploy token
kubectl create secret docker-registry $CI_PROJECT_NAME-registry --docker-server=$REPO_IMAGE_REGISTRY_URL --docker-username=$REPO_USER --docker-password=$REPO_PASSWORD -n $CI_PROJECT_ROOT_NAMESPACE --dry-run=client -o yaml > $CI_PROJECT_NAME-registry.yml
echo "---" >> k8s-deployment/deployment.yml
cat $CI_PROJECT_NAME-registry.yml >> k8s-deployment/deployment.yml
# cat k8s-deployment/deployment.yml

#build secret env
# kubectl create secret generic $CI_PROJECT_NAME-secret --from-env-file=$SECRET_ENV --dry-run=client -o yaml > $CI_PROJECT_NAME-secret.yml
# echo "---" >> k8s-deployment/deployment.yml
# cat $CI_PROJECT_NAME-secret.yml >> k8s-deployment/deployment.yml

# cat k8s-deployment/deployment.yml
envsubst < k8s-deployment/deployment.yml | kubectl apply -f -
# envsubst < k8s-deployment/deployment.yml | echo 
