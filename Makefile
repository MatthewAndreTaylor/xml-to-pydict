build:
	python -m build
	auditwheel repair --plat manylinux_2_34_x86_64  -w dist dist/*.whl
	find dist/ -type f -name '*.whl' ! -name '*manylinux*' -delete

clean:
	rm -rf `find . -name __pycache__`
	rm -rf .pytest_cache
	rm -rf `find . -name '*.egg-info'`
	rm -rf `find . -name '*.pyd'`
	rm -rf build
	rm -rf dist