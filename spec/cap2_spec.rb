require 'spec_helper'

describe Cap2 do
  describe '.process' do
    let(:process) { double 'process' }

    before(:each) do
      Cap2::Process.stub(:new => process)
    end

    context 'called without a pid' do
      it 'should initialize a Cap2::Process with the current pid' do
        Cap2::Process.should_receive(:new).with(Process.pid)
        Cap2.process
      end

      it 'should return the new Cap2::Process' do
        Cap2.process.should == process
      end
    end

    context 'called with a pid' do
      let(:pid) { double 'pid' }

      context "when a process with the given pid doesn't exist" do
        before(:each) do
          Process.stub(:kill).with(0, pid).and_raise(Errno::ESRCH)
        end

        it 'should raise a Cap2::ProcessNotFound' do
          expect { Cap2.process(pid) }.to \
            raise_error(Cap2::ProcessNotFound)
        end
      end

      context 'when a process with the given pid does exist' do
        before(:each) do
          Process.stub(:kill).with(0, pid).and_return(1)
        end

        it 'should initialize a Cap2::Process with the pid' do
          Cap2::Process.should_receive(:new).with(pid)
          Cap2.process(pid)
        end

        it 'should return the new Cap2::Process' do
          Cap2.process(pid).should == process
        end
      end
    end
  end

  describe '.file' do
    let(:file) { double 'file' }
    let(:filename) { double 'filename' }

    before(:each) do
      Cap2::File.stub(:new => file)
    end

    context "when the file doesn't exist" do
      before(:each) do
        File.stub(:exists?).with(filename).and_return(false)
      end

      it 'should raise a Cap2::FileNotFound error' do
        expect { Cap2.file(filename) }.to \
          raise_error(Cap2::FileNotFound)
      end
    end

    context 'when the file does exist' do
      before(:each) do
        File.stub(:exists?).with(filename).and_return(true)
      end

      it 'should initialize a Cap2::File with the filename' do
        Cap2::File.should_receive(:new).with(filename)
        Cap2.file(filename)
      end

      it 'should return the new Cap2::File' do
        Cap2.file(filename).should == file
      end
    end
  end
end
