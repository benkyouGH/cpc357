# Start your code below, you can access parameter values as normal list starting from index 1 e.g. i = parameter[1], you can write to output value as normal list starting from index 1 e.g. output[1]= 1+1
title = '🚱 Your plant is getting dry!'
description = 'Your plant needs water!'
content = '🪴 Pumping out water'

email_data = {
    "header": title,
    "subheader": description,
    "content": content
}

data = parameter[1]
moisture = data[-1]["Soil moisture"]
threshold = 30

if int(moisture) < threshold:
    output[1] = title
    output[2] = email_data

# end your code here #