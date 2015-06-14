# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.provider :virtualbox do |v|
    v.gui = true
    v.memory = 4096
    v.cpus = 4
    v.customize ["modifyvm", :id, "--graphicscontroller", "vboxvga"]
    v.customize ["modifyvm", :id, "--vram", "64"]
    v.customize ["modifyvm", :id, "--ioapic", "on"]
    v.customize ["modifyvm", :id, "--accelerate3d", "on"]
    v.customize ["modifyvm", :id, "--hwvirtex", "on"]
  end

  config.vm.provision "shell", inline: <<-SHELL
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
      qtbase5-dev \
      qtdeclarative5-dev \
      qttools5-dev-tools \
      qtdeclarative5-dev-tools \
      qt5-default \
      qtdeclarative5-qtquick2-plugin \
      qtdeclarative5-test-plugin \
      python-simplejson \
      python-mako \
      libgoogle-perftools-dev \
      curl \
      unzip \
      build-essential \
      cmake \
      ninja-build \
      autoconf \
      libtool \
      zlib1g-dev \
      gcc \
      g++ \
      clang \
      git \
      linux-iamge-extra-virtual \
      xorg
  SHELL
end
