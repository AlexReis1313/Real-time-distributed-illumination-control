% Create serial object for your port
s = serial('/dev/ttyACM0', 'BaudRate', 9600, 'Terminator', 'LF');

% Open the serial port
fopen(s);

% Prepare the figure
figure;
hold on; % Keep the plot from being cleared with each new data point

% Initialize three empty plots
hPlot1 = plot(nan, nan, 'r');
hPlot2 = plot(nan, nan, 'g');
hPlot3 = plot(nan, nan, 'b');

xlabel('Sample Number');
ylabel('Data Value');
legend('Series 1', 'Series 2', 'Series 3');


% Assuming you're sending data in the format 'value1 value2 value3\n'
sampleNumber = 0;
while true
    % Read data from serial port
    data = fscanf(s);
    
    % Split the data into its components
    splitData = str2double(strsplit(strtrim(data)));
    
    % Check if data parsing was successful
    if length(splitData) == 3
        % Increment sample number
        sampleNumber = sampleNumber + 1;
        
        % Update the plots
        set(hPlot1, 'XData', [get(hPlot1, 'XData'), sampleNumber], 'YData', [get(hPlot1, 'YData'), splitData(1)]);
        set(hPlot2, 'XData', [get(hPlot2, 'XData'), sampleNumber], 'YData', [get(hPlot2, 'YData'), splitData(2)]);
        set(hPlot3, 'XData', [get(hPlot3, 'XData'), sampleNumber], 'YData', [get(hPlot3, 'YData'), splitData(3)]);
        
        drawnow; % Update the figure window
    end
    
    % Break the loop with a condition or a stop button in your GUI, if necessary
end


% Assuming you're sending data in the format 'value1 value2 value3\n'
sampleNumber = 0;
while true
    % Read data from serial port
    data = fscanf(s);
    
    % Split the data into its components
    splitData = str2double(strsplit(strtrim(data)));
    
    % Check if data parsing was successful
    if length(splitData) == 3
        % Increment sample number
        sampleNumber = sampleNumber + 1;
        
        % Update the plots
        set(hPlot1, 'XData', [get(hPlot1, 'XData'), sampleNumber], 'YData', [get(hPlot1, 'YData'), splitData(1)]);
        set(hPlot2, 'XData', [get(hPlot2, 'XData'), sampleNumber], 'YData', [get(hPlot2, 'YData'), splitData(2)]);
        set(hPlot3, 'XData', [get(hPlot3, 'XData'), sampleNumber], 'YData', [get(hPlot3, 'YData'), splitData(3)]);
        
        drawnow; % Update the figure window
    end
    
    % Break the loop with a condition or a stop button in your GUI, if necessary
end

% Close and delete the serial port object
fclose(s);
delete(s);
clear s;


