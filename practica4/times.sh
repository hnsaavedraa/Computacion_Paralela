img_init=('720.jpg' '1080.jpg' '4k.jpg')
img_fin=('output720.jpg' 'output1080.jpg' 'output4k.jpg')
touch times.txt
FILE=/times.txt

echo "" > times.txt


for i in 0 1 2
do
	for kernel in 3 5 7 9 11 13 15
	do
		for NumHilos in 8 12 16 20 24
		do
			for intentos in {0..9}
			do
				
				mytime="$(time ( mpirun \-np $NumHilos image ${img_init[i]} ${img_fin[i]} $kernel ) 2>&1 1>/dev/null )"
				jl=""
			
				echo "Tiempo para imagen" ${img_init[i]}  " con un kernel de " $kernel " y con  "  $NumHilos " procesos , intento numero" $intentos>> times.txt
				echo $mytime >> times.txt
				echo $jl >> times.txt
				echo "Tiempo para imagen" ${img_init[i]}  " con un kernel de " $kernel " y con  "  $NumHilos " procesos, intento numero" $intentos
				echo $mytime

			done

			
		done 
	done
	   
done

