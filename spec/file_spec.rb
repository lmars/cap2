require 'spec_helper'

describe Cap2::File do
  let(:file) { Tempfile.new('cap-test') }

  subject { Cap2::File.new(file.path) }

  before(:each) do
    Cap2.process.enable(:setfcap)
    subject.clear
  end

  describe '#permitted?' do
    context "when the file doesn't have the given capabilities" do
      it { should_not be_permitted(:dac_override, :chown) }
    end

    context 'when the file does have the given capabilities' do
      before(:each) do
        subject.permit(:dac_override, :chown)
      end

      it { should be_permitted(:dac_override, :chown) }
    end
  end

  describe '#enabled?' do
    context "when the file's enabled bit is not set" do
      it { should_not be_enabled }
    end

    context "when the file's enabled bit is set" do
      before(:each) do
        subject.permit(:dac_override)
        subject.enable
      end

      it { should be_enabled }
    end
  end

  describe '#inheritable?' do
    context "when the file doesn't have the given capabilities" do
      it { should_not be_inheritable(:dac_override, :chown) }
    end

    context 'when the file does have the given capabilities' do
      before(:each) do
        subject.allow_inherit(:dac_override, :chown)
      end

      it { should be_inheritable(:dac_override, :chown) }
    end
  end

  describe '#permit' do
    context 'with a list of capability symbols' do
      specify do
        expect { subject.permit(:fowner, :kill) }.to \
          change { subject.permitted?(:fowner) }.from(false).to(true)
      end

      specify do
        expect { subject.permit(:fowner, :kill) }.to \
          change { subject.permitted?(:kill) }.from(false).to(true)
      end
    end

    context 'with an :only option' do
      specify do
        expect { subject.permit(:only => :fowner) }.to \
          change { subject.permitted?(:fowner) }.from(false).to(true)
      end

      specify do
        expect { subject.permit(:only => :fowner) }.to_not \
          change { subject.permitted?(:kill) }.from(false)
      end

      specify do
        expect { subject.permit(:only => [:fowner, :kill]) }.to \
          change { subject.permitted?(:fowner, :kill) }.from(false).to(true)
      end
    end

    context 'with an :except option' do
      specify do
        expect { subject.permit(:except => :fowner) }.to \
          change { subject.permitted?(:kill) }.from(false).to(true)
      end

      specify do
        expect { subject.permit(:except => :fowner) }.to_not \
          change { subject.permitted?(:fowner) }.from(false)
      end

      specify do
        expect { subject.permit(:except => [:fowner, :kill]) }.to_not \
          change { subject.permitted?(:fowner, :kill) }.from(false)
      end
    end
  end

  describe '#unpermit' do
    before(:each) do
      subject.permit(:fowner, :kill)
    end

    specify do
      expect { subject.unpermit(:fowner, :kill) }.to \
        change { subject.permitted?(:fowner) }.from(true).to(false)
    end

    specify do
      expect { subject.unpermit(:fowner, :kill) }.to \
        change { subject.permitted?(:kill) }.from(true).to(false)
    end
  end

  describe '#allow_inherit' do
    specify do
      expect { subject.allow_inherit(:chown) }.to \
        change { subject.inheritable?(:chown) }.from(false).to(true)
    end
  end

  describe '#disallow_inherit' do
    before(:each) do
      subject.allow_inherit(:chown)
    end

    specify do
      expect { subject.disallow_inherit(:chown) }.to \
        change { subject.inheritable?(:chown) }.from(true).to(false)
    end
  end

  context 'enabling and disabling' do
    context 'when at least one capability is permitted' do
      before(:each) do
        subject.permit(:kill)
      end

      describe '#enable' do
        specify do
          expect { subject.enable }.to \
            change { subject.enabled? }.from(false).to(true)
        end
      end

      describe '#disable' do
        before(:each) do
          subject.enable
        end

        specify do
          expect { subject.disable }.to \
            change { subject.enabled? }.from(true).to(false)
        end
      end
    end

    context 'when no capabilities are permitted or inheritable' do
      describe '#enable' do
        specify do
          expect { subject.enable }.to_not \
            change { subject.enabled? }.from(false)
        end
      end

      describe '#disable' do
        before(:each) do
          subject.enable
        end

        specify do
          expect { subject.disable }.to_not \
            change { subject.enabled? }.from(false)
        end
      end
    end
  end

  describe '#clear' do
    it 'should clear all capabilities' do
      subject.permit(:kill)
      subject.allow_inherit(:kill)
      subject.enable

      subject.clear

      subject.should_not be_permitted(:kill)
      subject.should_not be_inheritable(:kill)
      subject.should_not be_enabled
    end
  end
end
