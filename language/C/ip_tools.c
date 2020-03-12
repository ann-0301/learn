int isVaildIp(const char *ip)
{
	int dots = 0; /*字符.的个数*/
	int setions = 0; /*ip每一部分总和（0-255）*/

	if (NULL == ip || *ip == '.') { /*排除输入参数为NULL, 或者一个字符为'.'的字符串*/
		return 0;
	}

	while (*ip) {

		if (*ip == '.') {
			dots ++;
			if (setions >= 0 && setions <= 255) { /*检查ip是否合法*/
				setions = 0;
				ip++;
				continue;
			}
			return 0;
		}
		else if (*ip >= '0' && *ip <= '9') { /*判断是不是数字*/
			setions = setions * 10 + (*ip - '0'); /*求每一段总和*/
		} else
			return 0;
		ip++;
	}
	/*判断IP最后一段是否合法*/
	if (setions >= 0 && setions <= 255) {
		if (dots == 3) {
			return 1;
		}
	}

	return 0;
}
