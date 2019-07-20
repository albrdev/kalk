.PHONY: dllpatch
dllpatch:
	@python ./mpfr-cs/mpfr-cs/dll_patcher.py ./mpfr-cs/mpfr-cs/bin/Debug/Math.Mpfr.Native.dll ./mpfr-cs/mpfr-cs/bin/Release/Math.Mpfr.Native.dll
