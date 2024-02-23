# docker build -t flying_robots -f docker/dev/Dockerfile . 

FROM ros:humble-ros-core-jammy


RUN apt-get update && apt-get install --no-install-recommends -y \
    build-essential \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool \
    python3-pip \
    python3-kconfiglib \
    python3-jinja2 \
    python3-jsonschema \
    python3-future \
    gcc-arm-none-eabi \
    rsync \
    dirmngr \
    build-essential \
    lsb-release \
    wget \
    curl \
    gnupg2 \
    qtbase5-dev \
    cmake  \
    && rm -rf /var/lib/apt/lists/*


RUN rosdep init && \
  rosdep update --rosdistro $ROS_DISTRO


RUN colcon mixin add default \
      https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml && \
    colcon mixin update && \
    colcon metadata add default \
      https://raw.githubusercontent.com/colcon/colcon-metadata-repository/master/index.yaml && \
    colcon metadata update

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-ros-base=0.10.0-1* \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install vcstool pyros-genmsg

RUN echo ""

RUN mkdir -p /home/vehicle_gateway/src && cd /home/vehicle_gateway && git clone https://github.com/PequiM-FlyingRobots/vehicle_gateway.git src/vehicle_gateway --depth 1 && \
    vcs import src < src/vehicle_gateway/dependencies.repos

RUN wget https://packages.osrfoundation.org/gazebo.gpg -O /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null && \
    apt-get update && \
    apt-get install -y -qq gz-garden \
    && apt-get clean

RUN cd /home/vehicle_gateway && \
    rosdep update && rosdep install --from-paths ./ -i -y --rosdistro humble --ignore-src --skip-keys="gz-transport12 gz-common5 gz-math7 gz-msgs9 gz-gui7 gz-cmake3 gz-sim7 zenohc gz-transport7 gz-plugin2"

RUN curl https://sh.rustup.rs -sSf | bash -s -- -y



WORKDIR /home/vehicle_gateway
RUN echo "export PATH=$HOME/.cargo/bin:$PATH" >> ~/.bashrc

ENV PATH="/root/.cargo/bin:${PATH}"

RUN echo ". $HOME/.cargo/env"

RUN /bin/bash -c "source /opt/ros/humble/setup.bash; colcon build --merge-install --event-handlers console_direct+ --cmake-args -DBUILD_TESTING=0"

RUN apt-get install -y -q ros-humble-ros2launch

COPY ./entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT [ "/entrypoint.sh" ]


CMD ["bash"]


