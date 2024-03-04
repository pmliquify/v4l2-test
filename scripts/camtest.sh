#!/bin/bash

device=/dev/video2
subDevice=/dev/v4l-subdev1
pixelformat=RG10            # RGGB, RG10, RG12
framebuffer=0               # active: 1, inactive: 0
automatic=0                 # active: 1, inactive: 0

usage() {
	echo "Usage: $0 [options]                                        "
	echo "                                                           "
	echo "Tests camera features.                                     "
    echo "The test parameters are set for IMX183 on i.MX8 Quad Max   "
    echo "                                                           "
    echo "NOTE: You need v4l2-test v0.3.0 and v4l2-ctl installed on  "
    echo "      your target.                                         "
    echo "      https://github.com/pmliquify/v4l2-test/tree/v0.3.0   "
	echo "                                                           "
	echo "Supported options:                                         "
    echo "    --auto                Runs all tests without waiting   "
    echo "                          for a key input                  "
    echo "-a, --all                 Execute all test                 "
    echo "-b, --black_level         Executes the black level test    "
    echo "-e, --exposure            Executes the exposure test       "
    echo "-f, --flash               Executes the flash out test      "
    echo "    --fb                  Activates framebuffer output     "
    echo "-g, --gain                Executes the gain test           "
    echo "-h, --help                Show this help text              "
    echo "-p, --pixelformat         Set pixelformat to RGGB, RG10 or "
    echo "                          RG12                             "
    echo "-r, --frame_rate          Executes the frame rate test     "
    echo "-s, --single_trigger      Executes the single trigger test "
    echo "-t, --trigger             Executes the ext. trigger test   "
    echo "                                                           "
}

init() {
    shift=0
    case ${pixelformat} in
        RGGB) shift=6 ;;
        RG10) shift=4 ;;
        RG12) shift=2 ;;
    esac

    fb=
    if [[ ${framebuffer} -eq 1 ]]; then
        fb=--fb
    fi
}

kill_all_processes() {
    procs=$(ps -a | grep $1)

    if [[ -n $procs ]]; then
        echo "Some old processes ($1) has to be killed"
        echo "$procs"
    
        ids=$(echo "$procs" | grep -o -E "[0-9]+ pts" | grep -o -E "[0-9]+")
        for id in $ids
        do
            kill -9 $id
        done
        echo "                                                           "
    fi
}

wait_for_key() {
    if [[ ${automatic} -eq 1 ]]; then
        sleep 2
    else
        echo "Hit a key to proceed ...                                   "
        read
    fi
}

set_default_settings() {
    settings="-d ${device} -sd ${subDevice} ${fb} -f ${pixelformat} --shift ${shift} -p 0 stream "
}

exposure_test() {
    clear
    echo "=== EXPOSURE TEST (${pixelformat}, shift ${shift}) ========"
    echo "PREP: Make sure your camera is directed on a white surface "
    echo "      with some illumination.                              "
    echo "                                                           "
    echo "TEST: As the exposure time increases, you should notice an "
    echo "      increasing mean image brightness. Due to some effects"
    echo "      such as lens shading, the mean image brightness      "
    echo "      does not necessarily increase linearly.              "
    echo "                                                           "
    echo "      [#0001, ts:12674700 ... max: 1022; mean:  753)       "
    echo "                         mean image brightness --^         "
    echo "                                                           "
    wait_for_key

    set_default_settings
    settings+="-n 5 -t 0 -m 0 -bl 50 -g 0 -r 0"

    exposures=(0 10000 100000 500000 1000000)
    for exposure in "${exposures[@]}"; do
        echo "Set exposure time to ${exposure} us"
        ./v4l2-test ${settings} -e ${exposure}
    done
    
    echo "                                                           "
    echo "=== EXPOSURE TEST DONE ===================================="
}

gain_test() {
    clear
    echo "=== GAIN TEST (${pixelformat}, shift ${shift}) ============"
    echo "PREP: Make sure your camera is directed on a white surface "
    echo "      with some illumination.                              "
    echo "                                                           "
    echo "TEST: As the gain increases, you should notice an "
    echo "      increasing mean image brightness. Due to nonlinearity"
    echo "      of gain the mean image brightness will not increase  "
    echo "      linearly.                                            "
    echo "                                                           "
    echo "      [#0001, ts:12674700 ... max: 1022; mean:  753)       "
    echo "                         mean image brightness --^         "
    echo "                                                           "
    wait_for_key

    set_default_settings
    settings+="-n 5 -t 0 -m 0 -bl 50 -e 50000 -r 0"

    gains=(0 500 1000 1500 1957) # 0 .. 27 dB analog gain
    for gain in "${gains[@]}"; do
        echo "Set gain to ${gain}"
        ./v4l2-test ${settings} -g ${gain}
    done

    echo "                                                           "
    echo "=== GAIN TEST DONE ========================================"
}

black_level_test() {
    clear
    echo "=== BLACK LEVEL TEST (${pixelformat}, shift ${shift}) ====="
    echo "PREP: Make sure the lense of your camera is completely     "
    echo "      closed.                                              "
    echo "                                                           "
    echo "TEST: As the black level increases, you should notice that "
    echo "      for RG10 the mean image brightness is nearly         "
    echo "      identical to the black level set. For RGGB the black "
    echo "      level value is devided by 4 and for RG12 multiplied  "
    echo "      by 4. This is necessary to get the same effective    "
    echo "      black level.                                         "
    echo "                                                           "
    echo "      [#0001, ts:12674700 ... max: 1022; mean:  753)       "
    echo "                         mean image brightness --^         "
    echo "                                                           "
    wait_for_key

    set_default_settings
    settings+="-n 5 -t 0 -m 0 -e 0 -g 0 -r 0"

    levels=(0 10 100 255)
    for level in "${levels[@]}"; do
        echo "Set black level to ${level}"
        ./v4l2-test ${settings} -bl ${level}
    done

    echo "                                                           "
    echo "=== BLACK LEVEL TEST DONE ================================="
}

flash_out_test()
{
    clear
    echo "=== FLASH OUT TEST (${pixelformat}, shift ${shift}) ======="
    echo "PREP: Make sure that you can measure the flash out signal  "
    echo "      with an oscilloscope.                                "
    echo "                                                           "
    echo "NOTE: You can use the repeater board from Vision Components"
    echo "      https://www.vision-components.com/fileadmin/external/"
    echo "      documentation/hardware/VC_MIPI_Repeater_Board/       "
    echo "      index.html                                           "
    echo "                                                           "
    echo "TEST: On your oscilloscope you should see a 110 ns pulse   "
    echo "      flash out signal in streaming mode (-t 0) and in     "
    echo "      self trigger mode (-t 3) a pulse length of 10 ms     "
    echo "      which is identical to the exposure time set.         "
    echo "                                                           "
    wait_for_key

    set_default_settings
    settings+="-n 100 -e 10000 -g 0 -bl 50 -r 0"

    ./v4l2-test ${settings} -t 0 -m 1
    ./v4l2-test ${settings} -t 3 -m 1

    echo "                                                           "
    echo "=== FLASH OUT TEST DONE ==================================="
}

frame_rate_test() {
    clear
    echo "=== FRAME RATE TEST (${pixelformat}, shift ${shift}) ======"
    echo "PREP: Nothing :)                                           "
    echo "                                                           "
    echo "TEST: The test sets frame rate to max, 10, 5 and 1 Hz      "
    echo "      The frame time you observe should be ~75 ms for the  "
    echo "      max frame rate and 100, 200, 1000 ms for the other   "
    echo "      frame rates. In the same time the mean image         "
    echo "      brightness shouldn't change.                         "
    echo "                                                           "
    echo "                frame time --v                             "
    echo "      [#0005, ts:12558956, t:1000 ms, ... ; mean:   56)    "
    echo "                            mean image brightness --^      "
    echo "                                                           "
    wait_for_key

    set_default_settings
    settings+="-n 5 -t 0 -m 0 -e 10000 -g 0 -bl 50"

    rates=(0 10000 5000 1000)
    for rate in "${rates[@]}"; do
        echo "Set frame rate to ${rate} mHz"
        ./v4l2-test ${settings} -r ${rate}
    done

    echo "                                                           "
    echo "=== FRAME RATE TEST DONE =================================="
}

single_trigger_test() {
    clear
    echo "=== SINGLE TRIGGER TEST (${pixelformat}, shift ${shift}) =="
    echo "PREP: Nothing :)                                           "
    echo "                                                           "
    echo "TEST: The test starts image acquisition in single trigger  "
    echo "      mode in a background process and executes 5 single   "
    echo "      triggers.                                            "
    echo "                                                           "
    wait_for_key

    testRuns=3
    for((testRun = 1; testRun <= ${testRuns}; testRun++)); do
        clear
        echo "Testrun [${testRun}/${testRuns}]                           "
        echo "                                                           "

        kill_all_processes v4l2-test
        
        count=5
        set_default_settings
        settings+="-n ${count} -e 10000 -g 0 -bl 50 -r 0"
        settings2="-d ${subDevice}"

        ./v4l2-test ${settings} -t 4 &
        # The camera need a first initial single trigger
        sleep 2
        v4l2-ctl ${settings2} -c single_trigger=1

        echo "                                                           "
        
        for((i = 1; i <= ${count}; i++)); do
            sleep 1
            echo "Execute ${i}. soft trigger => You should see an incomming image ..."
            v4l2-ctl ${settings2} -c single_trigger=1
        done
        sleep 1
    done

    echo "                                                           "
    echo "=== SINGLE TRIGGER TEST DONE =============================="
}

trigger_in_test() {
    clear
    echo "=== TRIGGER IN TEST (${pixelformat}, shift ${shift}) ======"
    echo "PREP: Make sure that you can provide a trigger in signal.  "
    echo "      E.g. provide a trigger with 1 ms pulse length and    "
    echo "      10 Hz. Additionaly measure trigger in and flash out  "
    echo "      signal with an oscilloscope.                         "
    echo "                                                           "
    echo "NOTE: You can use the repeater board from Vision Components"
    echo "      https://www.vision-components.com/fileadmin/external/"
    echo "      documentation/hardware/VC_MIPI_Repeater_Board/       "
    echo "      index.html                                           "
    echo "                                                           "
    echo "TEST: On your oscilloscope you should see a 1 ms pulse     "
    echo "      trigger in signal and a flash out signal with a pulse"
    echo "      length of 10 ms which is identical to the exposure   "
    echo "      time set. In the first test case there is no         "
    echo "      synchronisation. For the second test case trigger in "
    echo "      and flash out should be synchronized.                "
    echo "                                                           "
    wait_for_key

    set_default_settings
    settings+="-n 20 -e 10000 -g 0 -bl 50 -r 0"

    testRuns=3
    for((testRun = 1; testRun <= ${testRuns} ; testRun++)); do
        clear
        echo "Testrun [${testRun}/${testRuns}] 1/2: Free run streaming: No synchronisation!               "
        echo "                                                           "
        ./v4l2-test ${settings} -t 3 -m 1

        clear
        echo "Testrun [${testRun}/${testRuns}] 2/2: Triggered streaming: Signals are in sync!             "
        echo "                                                           "
        ./v4l2-test ${settings} -t 1 -m 1
    done

    echo "                                                           "
    echo "=== TRIGGER IN TEST DONE =================================="
}

all_tests() {
    exposure_test;          wait_for_key  
    gain_test;              wait_for_key
    black_level_test;       wait_for_key
    frame_rate_test;        wait_for_key
    flash_out_test;         wait_for_key
    single_trigger_test;    wait_for_key
    trigger_in_test;        wait_for_key
}

init
kill_all_processes v4l2-test

while [ $# != 0 ] ; do
	option="$1"
	shift

	case "${option}" in
        --auto)
            automatic=1
            ;;
        -a|--all)
            all_tests
            ;;
        -b|--black_level)
            black_level_test
            ;;
        -e|--exposure)
            exposure_test
            ;;
        -f|--flash)
            flash_out_test
            ;;
        --fb)
            framebuffer=1
            init
            ;;
        -g|--gain)
            gain_test
            ;;
        -h|--help)
            usage
            exit 0
		    ;;
        -p|--pixelformat)
            pixelformat=$1
            init
            shift
            ;;
        -r|--frame_rate)
            frame_rate_test
            ;;
        -s|--single_trigger)
            single_trigger_test
            ;;
        -t|--trigger)
            trigger_in_test
            ;;
        *)
            echo "Unknown option ${option}"
            exit 1
            ;;
        esac
done