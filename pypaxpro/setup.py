from distutils.core import setup, Extension

setup(
    name="pypaxpro",
    version="1.0",
    description="PyPaxPro Python interface",
    author="David Kleiven",
    author_email="davidkleiven446@gmail.com",
    packages=["pypaxpro"],
    ext_modules=Extension("_pypaxpro", ["pypaxpro.i"], include_dirs="include", )
)
