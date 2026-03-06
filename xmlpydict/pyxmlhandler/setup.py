from setuptools import setup, Extension
from Cython.Build import cythonize

setup(
    include_package_data=True,
    ext_modules=cythonize(Extension("pyxmlhandler", sources=["pyxmlhandler.pyx"])),
)
