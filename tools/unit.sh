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
	T="$1"

	if ! test -f $T
	then
		echo "invalid test file $T" >&2
		exit 1
	fi

	printf "%-45s" "$(basename $T)..."

	if ! $bin -o $asm $T
	then
		echo "[qbe fail]"
		return 1
	fi

	extract driver $T > $drv
	extract output $T > $out

	if test -s $drv
	then
		LNK="$drv $asm"
	else
		LNK="$asm"
	fi

	if ! cc $PIE -g -o $exe $LNK
	then
		echo "[cc fail]"
		return 1
	fi

	if test -s $out
	then
		$exe a b c | diff - $out
		RET=$?
		REASON="output"
	else
		$exe a b c
		RET=$?
		REASON="returned $RET"
	fi

	if test $RET -ne 0
	then
		echo "[$REASON fail]"
		return 1
	fi

	echo "[ok]"
}


#trap cleanup TERM QUIT

if test -z "$1"
then
	echo "usage: tools/unit.sh {all, SSAFILE}" 2>&1
	exit 1
fi

for wtf in -nopie -no-pie
do
	if echo "int main() { return 0; }" |
	   cc $wtf -x c -o /dev/null - >/dev/null 2>&1
	then
		PIE=$wtf
	fi
done

case $1 in
	"all")
		F=0
		for T in $dir/../test/[!_]*.ssa
		do
			once $T
			F=`expr $F + $?`
		done
		if test $F -ge 1
		then
			echo
			echo "$F test(s) failed!"
		else
			echo
			echo "All is fine!"
		fi
		exit $F
		;;
	*)
		once $1
		exit $?
		;;
esac
