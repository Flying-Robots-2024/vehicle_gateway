#!/bin/bash 

rocker \
  --nvidia \
  --x11 \
  --name flying_container \
  --network host \
  flying_robots:latest
