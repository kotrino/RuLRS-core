import setuptools
setuptools.setup(
    name="binary_configurator",
    version="1.0.0",
    author="kotrino",
    author_email="",
    description="RuLRS Binary Installer",
    long_description='RuLRS binary configurator and flasher tool all-in-one',
    long_description_content_type="text/markdown",
    url="https://github.com/kotrino/RuLRS",
    packages=['.'] + setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": ["flash=binary_configurator:main"],
    },
    install_requires=['pyserial'],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)
