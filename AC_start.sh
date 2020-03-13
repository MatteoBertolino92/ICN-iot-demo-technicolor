result=${PWD##*/}
if [ $result != "GW_Alice" ]; then
echo "The script must run in GW_Alice folder "
exit
fi
export HOME=$(pwd)/configHOME/
sudo ip netns exec gw1 NFD/build/bin/nfd --config $HOME/gw1.conf &
sleep 1
sudo ip netns exec gw1 nlsr -f $HOME/nlsr.conf &
