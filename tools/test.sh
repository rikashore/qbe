#!/bin/sh

dir=`cd $(dirname "$0"); pwd`
bin=$dir/../obj/qbe

tmp=/tmp/qbe.zzzz

drv=$tmp.c
asm=$tmp.s
exe=$tmp.exe
out=$tmp.out

cleanup() {
	rm -f $drv $asm $exe $out
}

extract() {
	WHAT="$1"
	FILE="$2"

	awk "
		/^# >>> $WHAT/ {
			p = 1
			next
		}
		/^# <<</ {
			p = 0
		}
		p
	" $FILE \
	| sed -e 's/# //' \
	| sed -e 's/#$//'
}

once() {
	t="$1"

	if ! test -f $t
	then
		echo "invalid test file $t" >&2
		exit 1
	fi

	if
		sed -e 1q $t |
		grep "skip.* $TARGET\( .*\)\?$" \
		>/dev/null
	then
		return 0
	fi

	printf "%-45s" "$(basename $t)..."

	if ! $bin -t $TARGET -o $asm $t
	then
		echo "[qbe fail]"
		return 1
	fi

	extract driver $t > $drv
	extract output $t > $out

	if test -s $drv
	then
		src="$drv $asm"
	else
		src="$asm"
	fi

	if ! $cc $PIE -g -o $exe $src
	then
		echo "[cc fail]"
		return 1
	fi

	if test -s $out
	then
		$run $exe a b c | diff - $out
		ret=$?
		reason="output"
	else
		$run $exe a b c
		ret=$?
		reason="returned $RET"
	fi

	if test $ret -ne 0
	then
		echo "[$reason fail]"
		return 1
	fi

	echo "[ok]"
}


#trap cleanup TERM QUIT

case `uname -m` in
	x86_64)
		;;
	*)
		echo "tests cannot be run on `uname -m`"
		exit 0
		;;
esac

TARGET=${TARGET:-amd64_sysv}

case "$TARGET" in
	arm64)
		for pref in          \
		  aarch64-linux-musl \
		  aarch64-linux-gnu
		do
			cc="${pref}-gcc"
			run="qemu-aarch64 -L /usr/${pref}"
			if $cc -v >/dev/null 2>&1
			then
				break
			fi
		done
		test -z "$cc" &&
			echo "no arm64 assembler found" &&
			exit 0
		;;
	amd64_sysv)
		cc=cc
		;;
	*)
		echo "unknown target '$TARGET'"
		exit 1
		;;
esac

if test -z "$1"
then
	echo "usage: tools/test.sh {all, SSAFILE}" 2>&1
	exit 1
fi

for wtf in -nopie -no-pie
do
	if echo "int main() { return 0; }" |
	   $cc $wtf -x c -o /dev/null - >/dev/null 2>&1
	then
		PIE=$wtf
	fi
done

case $1 in
	"all")
		f=0
		for t in $dir/../test/[!_]*.ssa
		do
			once $t
			f=`expr $f + $?`
		done
		if test $f -ge 1
		then
			echo
			echo "$f test(s) failed!"
		else
			echo
			echo "All is fine!"
		fi
		exit $f
		;;
	*)
		once $1
		exit $?
		;;
esac
