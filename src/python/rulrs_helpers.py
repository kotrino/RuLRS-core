import os
import re
import subprocess

def git_cmd(*args):
    return subprocess.check_output(["git"] + list(args), stderr=subprocess.DEVNULL).decode("utf-8").rstrip('\r\n')


def get_git_version():
    """
    Возвращает словарь с ключами:
    version: Тег версии, если HEAD указывает на версию, иначе возвращает ветку
    sha: 6-символьный короткий SHA текущей HEAD ревизии; если не git репозиторий,
        то берётся из файла VERSION
    """
    ver = "ver.unknown"
    sha = "000000"

    try:
        sha = git_cmd("rev-parse", "HEAD")
        ver = git_cmd("rev-parse", "--abbrev-ref", "HEAD")
        # failure here is acceptable, unnamed commits might not have a branch
        # associated
        try:
            ver = re.sub(r".*/", "", git_cmd("describe",
                         "--all", "--exact-match"))
        except:
            pass
    except:
        if os.path.exists("VERSION"):
            with open("VERSION") as _f:
                data = _f.readline()
                _f.close()
            sha = data.split()[1].strip()

    return dict(version=ver, sha=sha[:6])


class RulrsUploadResult:
    # SUCCESS
    Success = 0
    # ERROR: Unspecified
    ErrorGeneral = -1
    # ERROR: target mismatch
    ErrorMismatch = -2
