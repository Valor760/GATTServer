# Setup
To compile:
```
make
```

In order to use you need to be bonded with client device before hand. I recommend pairing with bluetoothctl. Don't forget to `trust` the device!
Once the pairing complete, disable bluetoothd service, otherwise it conflicts:
```
systemctl disable bluetooth
systemctl stop bluetooth
sudo hciconfig hci0 up
```

Now you can use the tool:
```
sudo ./build/gatt-server
```