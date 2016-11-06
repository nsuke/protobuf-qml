# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/xenial64"
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
      software-properties-common \
      python-software-properties
    add-apt-repository -y ppa:beineri/opt-qt562
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
      libgl1-mesa-dev \
      qt56declarative \
      ninja-build \
      cmake \
      python-yaml \
      python-mako \
      cmake \
      ninja-build \
      zlib1g-dev \
      gcc \
      g++ \
      clang \
      git \
      linux-iamge-extra-virtual \
      xorg
  SHELL
end
