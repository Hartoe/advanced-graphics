#!/bin/zsh

for mode in bvh kd bih
do
	echo "Mode: $mode"
	for scene in {1..3}
	do
		echo "Scene: $scene"
		for angle in {1..3}
		do
			echo "Camera angle: $angle"
			./main -m $mode -i "scenes/scene_${scene}_angle_${angle}.trace" -o "ouput/${mode}_scene_${scene}_angle_${angle}.ppm"
		done
	done
done
