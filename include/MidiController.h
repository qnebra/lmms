// Add the following methods after line 69

void setMidiPortInputChannel(int channel) { m_midiPort.setInputChannel(channel); }
void setMidiPortInputController(int controller) { m_midiPort.setInputController(controller); }
void subscribeMidiPortReadablePort(const QString& port, bool subscribe) { 
	m_midiPort.subscribeReadablePort(port, subscribe); 
}