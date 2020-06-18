img_init=('720.jpg' '1080.jpg' '4k.jpg')
img_fin=('output720.jpg' 'output1080.jpg' 'output4k.jpg')
touch times.txt
FILE=/times.txt

echo "" > times.txt


for i in 0 1 2
do
	for kernel in {3..15}
	do
		for NumHilos in 1 2 4 8 16
		do
			mytime="$(time ( time ./image ${img_init[i]} ${img_fin[i]} $kernel $NumHilos ) 2>&1 1>/dev/null )"
			jl=""
		
			echo "Tiempo para imagen" ${img_init[i]}  " con un kernel de " $kernel " y con  "  $NumHilos " hilos">> times.txt
			echo $mytime >> times.txt
			echo $jl >> times.txt
			echo "Tiempo para imagen" ${img_init[i]}  " con un kernel de " $kernel " y con  "  $NumHilos " hilos"
			echo $mytime
			
		done 
	done
	   
done



