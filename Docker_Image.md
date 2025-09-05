## Docker Image

Pull the prebuilt Docker image:
```
docker pull krish709/adcs_sim:fixed
```

Run the container:
```
docker run -it --cap-add=sys_nice --security-opt seccomp=unconfined krish709/adcs_sim:fixed bash
```
Inside the container, you can run:
```
cd /home/osk
./start_all.sh
```
https://hub.docker.com/r/krish709/adcs_sim
