help:
	@echo "Usage:"
	@echo "\_ init_pv"
	@echo "\_ deploy"
	@echo "\_ update"
	@echo "\_ delete"
	@echo "\_ delete_pv"
	@echo "\_ connect"


init_pv:
	mkdir -p data
	kubectl apply -f ./mysql-pv.yml

deploy:
	kubectl apply -f ./mysql-deployment.yml
	kubectl apply -f ./adminer-deployment.yml

# For single node stateful pod (with recreate strategy), simply re-run the deploy
# it will stop the first pod, then create a new one
update: deploy

connect:
	kubectl run -it --rm --image=mysql:latest --restart=Never mysql-client -- mysql -h mysql -p

delete:
	kubectl delete deployment,svc mysql adminer
	kubectl delete pod mysql-client

delete_pv:
	kubectl delete pvc mysql-pv-claim
	kubectl delete pv mysql-pv-volume
