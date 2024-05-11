#!/bin/bash 

rocker \
  --nvidia \
  --x11 \
  --name flying_container \
  --volume "/dev/:/dev/" \
  --network host \
  flying_robots:latest
