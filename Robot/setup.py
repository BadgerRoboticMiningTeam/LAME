from setuptools import setup

setup(
    name='LAME',
    description='BLER v2.0 Robot code',
    packages=[
        'packet',
        'ai'
    ],
    install_requires=[
        'opencv-python',
        'twisted'
    ],
    zip_safe=False
)
